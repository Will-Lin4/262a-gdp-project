/* Global Data Plane meta data.

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
import java.util.HashMap;
import java.util.Map;
import java.awt.PointerInfo;
import java.lang.Exception;


import com.sun.jna.Native;
import com.sun.jna.Memory;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.PointerByReference;
import org.terraswarm.gdp.NativeSize; // Fixed by cxh in makefile.

/**
 * Global Data Plane meta data.
 * 
 * @author Nitesh Mor
 */

public class GDP_MD {

    // See the C-API for an explanation of why these magic numbers were chosen.

    public static final int GDP_MD_XID          = 0x00584944; // XID (external id)
    public static final int GDP_MD_PUBKEY       = 0x00505542; // PUB (public key, deprecated)
    public static final int GDP_MD_OWNERPUBKEY  = 0x004F504B; // OPK (owner public key)
    public static final int GDP_MD_WRITERPUBKEY = 0x0057504B; // WPK (writer public key)
    public static final int GDP_MD_CTIME        = 0x0043544D; // CTM (creation time)
    public static final int GDP_MD_EXPIRE       = 0x0058544D; // XTM (expiration date/time)
    public static final int GDP_MD_CREATOR      = 0x00434944; // CID (creator id)
    public static final int GDP_MD_SYNTAX       = 0x0053594E; // SYN (data syntax: json, xml, etc.)
    public static final int GDP_MD_LOCATION     = 0x004C4F43; // LOC (location: lat/long)
    public static final int GDP_MD_NONCE        = 0x004E4F4E; // NON (unique nonce)

    
    /**
     * Pass a pointer to an already allocated gdp_md_t C structure.
     * @param d A pointer to an existing gdp_md_t
     */
    public GDP_MD(PointerByReference d) {
        this.gdp_md_ptr = d;
        this.did_i_create_it = false;
        return;
    }
    
    /**
     * If called without an existing pointer, we allocate a new 
     * C gdp_md_t structure.
     */
    public GDP_MD() {
        this.gdp_md_ptr = Gdp21Library.INSTANCE.gdp_md_new(0);
        this.did_i_create_it = true;
    }
 
    /**
     * If called without an existing pointer with non-zero entries,
     * we allocate a new C gdp_md_t structure with appropriate args.
     */
    public GDP_MD(int entries) {
        this.gdp_md_ptr = Gdp21Library.INSTANCE.gdp_md_new(entries);
        this.did_i_create_it = true;
    }
    
    /**
     * In case we allocated memory for the C gdp_md_t ourselves, free it.
     */
    public void finalize() {        
        if (this.did_i_create_it) {
            Gdp21Library.INSTANCE.gdp_md_free(this.gdp_md_ptr);
        }
    }
    
    /**
     * Add a new entry to the metadata
     * @param md_id  ID for the metadata. Internally a 32-bit unsigned integer
     * @param data  The corresponding data. We automatically calculate length
     *              and pass to the corresponding C routine.
     */
    public void add(int md_id, byte[] data) throws GDPException {

        EP_STAT estat;
        
        // FIXME: figure out if we need to have a +1 for len
        NativeSize len = new NativeSize(data.length);

        // Create a C string that is a copy (?) of the Java String.
        Memory memory = new Memory(data.length+1);
        
        // FIXME: not sure about alignment.
        Memory alignedMemory = memory.align(4);
        memory.clear();
        Pointer pointer = alignedMemory.share(0);

        for (int i=0; i<data.length; i++) {
            pointer.setByte(i,data[i]);
        }
                
        estat = Gdp21Library.INSTANCE.gdp_md_add(this.gdp_md_ptr, md_id,
                len, pointer);

        // don't forget error checking
        GDP.check_EP_STAT(estat, "GDP_MD.java: Adding metadata entry (" +
                                        md_id + ") failed.");
    }

    
    /**
     * Get a metadata entry by index. This is not searching by a md_id. For
     * that, look at find.
     * @param index The particular metadata entry we are looking for
     * @return  Returns a Key-Value pair containing the requested index. 
     *          A hashMap seems to be the best choice, since there's no tuple
     *          type in Java.
     */ 
    public HashMap<Integer, byte[]> get(int index) throws GDPException {

        EP_STAT estat;

        // Pointer to hold the md_id
        IntBuffer md_id_ptr = IntBuffer.allocate(1);
        
        // to hold the length of the buffer
        NativeSize len = new NativeSize();
        
        // Pointer to the buffer to hold the value
        PointerByReference p = new PointerByReference();
        
        estat = Gdp21Library.INSTANCE.gdp_md_get(this.gdp_md_ptr, 
                index, md_id_ptr, new NativeSizeByReference(len), p);

        GDP.check_EP_STAT(estat, "GDP_MD.java: gdp_md_get(" + index + ") failed.");
        
        // get the md_id
        int md_id = md_id_ptr.get(0);
        
        Pointer pointer = p.getValue();
        byte[] val = pointer.getByteArray(0, len.intValue());
        
        HashMap<Integer, byte[]> ret = new HashMap<Integer, byte[]>();
        
        ret.put(md_id, val);
        
        return ret;
    }
    
    /**
     * Find a particular metadata entry by looking up md_id
     * @param md_id ID we are searching for
     * @return the value associated with <code>md_id</code> 
     */
    public byte[] find(int md_id) throws GDPException {

        EP_STAT estat;

        // to hold the length of the buffer
        NativeSize len = new NativeSize();
        // Pointer to the buffer to hold the value
        PointerByReference p = new PointerByReference();
        
        estat = Gdp21Library.INSTANCE.gdp_md_find(this.gdp_md_ptr, 
                md_id, new NativeSizeByReference(len), p);

        GDP.check_EP_STAT(estat, "GDP_MD.java: gdp_md_find(" + md_id + ") failed.");
        
        Pointer pointer = p.getValue();
        byte[] bytes = pointer.getByteArray(0, len.intValue());
        return bytes;
        
    }

    // Pointer to the C structure
    public PointerByReference gdp_md_ptr = null;
    
    // To keep track of whether we need to call free in destructor
    private boolean did_i_create_it = false;


}
