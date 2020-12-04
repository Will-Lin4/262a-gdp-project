/* Global Data Plane hashes.

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
 * Global Data Plane hashes.
 * 
 * @author Nitesh Mor
 */

public class GDP_HASH {

    /**
     * Pass a pointer to an already allocated gdp_hash_t C structure.
     * @param d A pointer to an existing gdp_hash_t
     */
    public GDP_HASH(PointerByReference d) {
        this.gdp_hash_ptr = d;
        this.did_i_create_it = false;
        return;
    }
    
    /**
     * If called without an existing pointer, we allocate a new 
     * C gdp_hash_t structure.
     *
     * @param alg hash algorithm
     */
    public GDP_HASH(int alg) {
        this.gdp_hash_ptr = Gdp21Library.INSTANCE.gdp_hash_new(alg, null, new NativeSize(0));
        this.did_i_create_it = true;
    }
 
     /**
     * If called without an existing pointer, we allocate a new 
     * C gdp_hash_t structure.
     *
     * @param alg hash algorithm
     * @param hashbytes initial bytes
     */
    public GDP_HASH(int alg, byte[] hashbytes) {

        Memory memory = new Memory(hashbytes.length);
        Memory alignedMemory = memory.align(4);
        memory.clear();
        Pointer pointer = alignedMemory.share(0);

        for (int i=0; i<hashbytes.length; i++) {
            pointer.setByte(i, hashbytes[i]);
        }

        this.gdp_hash_ptr = Gdp21Library.INSTANCE.gdp_hash_new(
                        alg, pointer, new NativeSize(hashbytes.length));
        this.did_i_create_it = true;
    }
    
    /**
     * In case we allocated memory for the C gdp_hash_t ourselves, free it.
     */
    public void finalize() {        
        if (this.did_i_create_it) {
            Gdp21Library.INSTANCE.gdp_hash_free(this.gdp_hash_ptr);
        }
    }

    /////////////////////////////////////////////////////////////

    public void reset() {
        Gdp21Library.INSTANCE.gdp_hash_reset(this.gdp_hash_ptr);
    }

    public void set(byte[] hashbytes) {

        Memory memory = new Memory(hashbytes.length);
        Memory alignedMemory = memory.align(4);
        memory.clear();
        Pointer pointer = alignedMemory.share(0);

        for (int i=0; i<hashbytes.length; i++) {
            pointer.setByte(i, hashbytes[i]);
        }

        Gdp21Library.INSTANCE.gdp_hash_set(this.gdp_hash_ptr,
                        pointer, new NativeSize(hashbytes.length));
    }


    public int getlength() {
        return Gdp21Library.INSTANCE.gdp_hash_getlength(this.gdp_hash_ptr).intValue();
    }


    /**
     * Equivalent to gdp_hash_getptr in the C-API, except it returns bytes
     */
    // public byte[] getdata() {
        // TODO finish this implementation
    // }



    public boolean equal(GDP_HASH x) {

        byte ret = Gdp21Library.INSTANCE.gdp_hash_equal(
                            this.gdp_hash_ptr, x.gdp_hash_ptr);
        if (ret == 0x00) {
            return false;
        } else {
            return true;
        }
    }

    /////////////////////////////////////////////////////////////


    // Pointer to the C structure
    public PointerByReference gdp_hash_ptr = null;
    
    // To keep track of whether we need to call free in destructor
    private boolean did_i_create_it = false;


}
