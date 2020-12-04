/* Global Data Plane Open Info

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
 * Global Data Plane Open Info.
 * 
 * @author Nitesh Mor
 */

public class GDP_OPEN_INFO {

   
    /**
     * Pass a pointer to an already allocated gdp_open_info_t C structure.
     * @param d A pointer to an existing gdp_open_info_t
     */
    public GDP_OPEN_INFO(PointerByReference d) {
        this.gdp_open_info_ptr = d;
        this.did_i_create_it = false;
        return;
    }
    
    /**
     * If called without an existing pointer, we allocate a new 
     * C gdp_open_info_t structure.
     */
    public GDP_OPEN_INFO() {
        this.gdp_open_info_ptr = Gdp21Library.INSTANCE.gdp_open_info_new();
        this.did_i_create_it = true;
    }
 
    /**
     * In case we allocated memory for the C gdp_open_info_t ourselves, free it.
     */
    public void finalize() {        
        if (this.did_i_create_it) {
            Gdp21Library.INSTANCE.gdp_open_info_free(this.gdp_open_info_ptr);
        }
    }


    ////////////////////////////////////////////////////////////
    ////////// "set" functions for various parameters //////////
    ////////// See the corresponding C API for details. ////////
    ////////////////////////////////////////////////////////////
    
    /**
     * Set the signing key
     * @param skey
     */
    public void set_signing_key(EP_CRYPTO_KEY skey) {
        EP_STAT estat;
        estat = Gdp21Library.INSTANCE.gdp_open_info_set_signing_key(
                                    this.gdp_open_info_ptr, skey.key_ptr);
    }

    // TODO implement callback for reading a signing key from a user
    // TODO i.e. gdp_open_info_set_signkey_cb


    /**
     * Set caching behavior
     * @param keep_in_cache
     */ 
    public void set_caching(boolean keep_in_cache) {

        EP_STAT estat;
        byte _keep_in_cache = 0x00;
        if (keep_in_cache) {
            _keep_in_cache = 0x01;
        }
        estat = Gdp21Library.INSTANCE.gdp_open_info_set_caching(
                                    this.gdp_open_info_ptr, _keep_in_cache);
    }
    
    /**
     * Set data verification behavior
     * @param verify_proof
     */
    public void set_vrfy(boolean verify_proof) {

        EP_STAT estat;
        byte _verify_proof = 0x00;
        if (verify_proof) {
            _verify_proof = 0x01;
        }
        estat = Gdp21Library.INSTANCE.gdp_open_info_set_vrfy(
                                    this.gdp_open_info_ptr, _verify_proof);
    }


    ////////////////////////////////////////////////////////////

    // Pointer to the C structure
    public PointerByReference gdp_open_info_ptr = null;
    
    // To keep track of whether we need to call free in destructor
    private boolean did_i_create_it = false;


}
