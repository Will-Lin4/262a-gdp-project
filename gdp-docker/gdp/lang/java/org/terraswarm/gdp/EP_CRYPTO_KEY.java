/* Global Data Plane Crypto key management

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
 * Global Data Plane Crypto key management
 * 
 * @author Nitesh Mor
 */

public class EP_CRYPTO_KEY {


    public static final int EP_CRYPTO_F_PUBLIC = 0x0000;
    public static final int EP_CRYPTO_F_SECRET = 0x0001;

   
    /**
     * Pass a pointer to an already allocated C structure.
     * @param d
     */
    public EP_CRYPTO_KEY(PointerByReference d) {
        this.key_ptr = d;
        this.did_i_create_it = false;
        return;
    }
    
    /**
     * If called without an existing pointer, we probably are trying to
     * create a new key
     *
     * @param keytype
     * @param keylen
     * @param keyep
     * @param curve
     */
    public EP_CRYPTO_KEY(int keytype, int keylen, int keyexp, String curve) {
        this.key_ptr = Gdp21Library.INSTANCE.ep_crypto_key_create(
                                        keytype, keylen, keyexp, curve);
        this.did_i_create_it = true;
    }
 

    /**
     * Read from a file if we are provided with a filename
     * 
     * @param filename
     * @param keyform
     * @param flags
     */

    public EP_CRYPTO_KEY(String filename, int keyform, int flags) {
        this.key_ptr = Gdp21Library.INSTANCE.ep_crypto_key_read_file(
                            filename, keyform, flags);
        this.did_i_create_it = true;
    }

    // TODO add routines for loading the key from a file pointer or memory


    /**
     * In case we allocated memory for the C structure ourselves, free it.
     */
    public void finalize() {        
        if (this.did_i_create_it) {
            Gdp21Library.INSTANCE.ep_crypto_key_free(this.key_ptr);
        }
    }


    ////////////////////////////////////////////////////////////

    /**
     * Write a key, that presumably was created by us, to a file
     *
     * @param filename
     * @param keyform
     * @param keyenc
     * @param passwd
     * @param flags
     */
    public void write_file(String filename, int keyform, int keyenc,
                                String passwd, int flags) {

        Gdp21Library.INSTANCE.ep_crypto_key_write_file(this.key_ptr,
                                filename, keyform, keyenc, passwd, flags);
    }


    ////////////////////////////////////////////////////////////

    // Pointer to the C structure
    public PointerByReference key_ptr = null;
    
    // To keep track of whether we need to call free in destructor
    private boolean did_i_create_it = false;


}
