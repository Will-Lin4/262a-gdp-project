/* Names inside the Global Data Plane.

   Copyright (c) 2015-2016 The Regents of the University of California.

   All rights reserved.
   Permission is hereby granted, without written agreement and without
   license or royalty fees, to use, copy, modify, and distribute this
   software and its documentation for any purpose, provided that the above
   copyright notice and the following two paragraphs appear in all copies
   of this software.

   IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
   FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
   ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
   THE UNIVERSITY OF CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF
   SUCH DAMAGE.

   THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE
   PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
   CALIFORNIA HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
   ENHANCEMENTS, OR MODIFICATIONS.

   PT_COPYRIGHT_VERSION_2
   COPYRIGHTENDKEY

 */

package org.terraswarm.gdp; 

import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.util.Date;
import java.util.Map;
import java.awt.PointerInfo;
import java.lang.Exception;


import com.sun.jna.Native;
import com.sun.jna.Memory;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.PointerByReference;
import org.terraswarm.gdp.NativeSize; // Fixed by cxh in makefile.

/**
 * Names inside the GDP. This works for GIN's, log-servers
 * and any other entities. An entity can have up to three kind of names:
 *
 * <ul>
 * <li> A 256 bit binary name </li>
 * <li> A base-64 (-ish) representation </li>
 * <li> (optional) A more memorable name, which is potentially hashed to
 * generate the 256-bit binary name </li>
 * </ul>
 * 
 * @author Nitesh Mor
 */
public class GDP_NAME {
    
    /**
     * Pass a string that could be either a human readable name, or a
     * printable name.
     * @param name A version of the name
     * @exception GDPException If a GDP C function returns a code great than or equal to Gdp21Library.EP_STAT_SEV_WARN.
     */
    public GDP_NAME(String name) throws GDPException {
        this.name = this.__parse_name(name);
        this.pname = this.__get_printable_name(this.name);
    }

    /**
     * Pass a binary string containing the internal name
     * @param name the internal name
     */
    public GDP_NAME(byte[] name) {
        assert name.length == 32;
        this.name = name;
        this.pname = this.__get_printable_name(name);
    }
    
    /**
     * Get the printable name
     * @return printable name as a byte array
     */
    public byte[] printable_name() {
        // We need to make a copy.
        return this.pname.clone();
    }

    /**
     * Get the internal 256-bit name
     * @return internal name as a byte array. length: 32 bytes
     */
    public byte[] internal_name() {
        // We need to make a copy.
        return this.name.clone();
    }

    /**
     * Some sanity checking, borrowed from the C-api
     * @return whether this is a valid GDP name or not
     */
    public boolean is_valid() {
        ByteBuffer tmp = ByteBuffer.allocate(32);

        // Copy to newly created bytebuffer.
        tmp.put(this.name);

        byte ret = Gdp21Library.INSTANCE.gdp_name_is_valid(tmp);
        if (ret == 0x00) {
            return false;
        } else {
            return true;
        }
    }


    public static byte[] resolve(String hname) {
        EP_STAT estat;
        ByteBuffer gname = ByteBuffer.allocate(32);
        estat = Gdp21Library.INSTANCE.gdp_name_resolve(hname, gname);
        return gname.array();
    }

    // XXX not sure what this is supposed to do. Not implemented
    // in C interface yet. I am guessing this is to update the
    // human name to GDP name mapping. Marking this as static,
    // since that doesn't have any reliance on a specific GDP_NAME
    // object.
    public static void update(String hname, byte[] gname) {
        byte[] gname_copy = gname.clone();
        ByteBuffer tmp = ByteBuffer.wrap(gname_copy);
        Gdp21Library.INSTANCE.gdp_name_update(hname, tmp);
    }

    public void root_set(String root) {
        Gdp21Library.INSTANCE.gdp_name_root_set(root);
    }

    public String root_get() {
        return Gdp21Library.INSTANCE.gdp_name_root_get();
    }

    /////////////// PRIVATE MEMBERS ///////////////////////
    
    // to hold the 256-bit binary name
    private byte[] name = new byte[32];
    
    private static int PNAME_LEN = 43;
    
    // to hold the printable name
    private byte[] pname = new byte[this.PNAME_LEN + 1];
    
    /**
     * Just a heurestic to check if a string is binary or not
     * <p>
     * We don't need this anymore. Should we cleanup???
     * @param s the string under question
     * @return yes if it contains non-text characters
     */
    private boolean __is_binary(byte[] s) {

        // textchars = {7,8,9,10,12,13,27} | set(range(0x20, 0x100)) - {0x7f}
        for (byte b: s) {
            int x = (int) b; // typecast
            
            if (!( (x>=7 && x<=10) || x==12 || x==13 || x==27 || 
                    (x>=0x20 && x<0x100 && x!=0x7f) )) 
                return true;
        }
        return false;
    }
    
    /**
     * Parse the (presumably) printable name to return an internal name
     * @param s A potential printable name
     * @return A 32-byte long internal name
     * @exception GDPException If a GDP C function returns a code great
     *                         than or equal to Gdp21Library.EP_STAT_SEV_WARN.     
     */
    private byte[] __parse_name(String s) throws GDPException {
        ByteBuffer dst = ByteBuffer.allocate(32); // FIXME
        
        EP_STAT estat = Gdp21Library.INSTANCE.gdp_parse_name(s, dst);
        GDP.check_EP_STAT(estat, "Failed to parse " + s);

        // we don't care if the bytebuffer gets modified. We aren't using
        // it elsewhere anyways.
        return dst.array();
    }
    
    /**
     * Generate printable name from internal name
     * @param internal_name
     * @return
     */
    private byte[] __get_printable_name(byte[] internal_name) {
        
        byte[] internal_name_copy = internal_name.clone();
        
        ByteBuffer internal = ByteBuffer.wrap(internal_name_copy);
        ByteBuffer printable = ByteBuffer.allocate(this.PNAME_LEN+1);

        Gdp21Library.INSTANCE.gdp_printable_name(internal, printable);
        // no problem, since we don't need bytebuffer interface to this
        // memory anymore.
        
        return printable.array();
    }

    /**
      * Generate internal name from printable name
      * @param printable_name
      * @return
      */
    private byte[] __get_internal_name(byte[] printable_name) {

        byte[] printable_name_copy = printable_name.clone();

        ByteBuffer internal = ByteBuffer.allocate(32);
        ByteBuffer printable = ByteBuffer.wrap(printable_name_copy);

        Gdp21Library.INSTANCE.gdp_internal_name(printable, internal);

        return internal.array();

    }
}
 
