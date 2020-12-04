/* Global Data Plane Create Info

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
 * Global Data Plane Create Info.
 * 
 * @author Nitesh Mor
 */

public class GDP_CREATE_INFO {

   
    /**
     * Pass a pointer to an already allocated gdp_create_info_t C structure.
     * @param d A pointer to an existing gdp_create_info_t
     */
    public GDP_CREATE_INFO(PointerByReference d) {
        this.gdp_create_info_ptr = d;
        this.did_i_create_it = false;
        return;
    }
    
    /**
     * If called without an existing pointer, we allocate a new 
     * C gdp_create_info_t structure.
     */
    public GDP_CREATE_INFO() {
        this.gdp_create_info_ptr = Gdp21Library.INSTANCE.gdp_create_info_new();
        this.did_i_create_it = true;
    }
 
    /**
     * In case we allocated memory for the C gdp_create_info_t ourselves, free it.
     */
    public void finalize() {        
        if (this.did_i_create_it) {
            Gdp21Library.INSTANCE.gdp_create_info_free(this.gdp_create_info_ptr);
        }
    }


    ////////////////////////////////////////////////////////////
    ////////// "set" functions for various parameters //////////
    ////////// See the corresponding C API for details. ////////
    ////////////////////////////////////////////////////////////
    
    /**
     * Set the creator (email?).
     * @param user
     * @param domain
     */
    public void set_creator(String user, String domain) {
        EP_STAT estat;
        estat = Gdp21Library.INSTANCE.gdp_create_info_set_creator(
                                    this.gdp_create_info_ptr, user, domain);
    }

    /**
     * Set expiration date, specified as 'seconds' from now.
     * @param expiration
     */ 
    public void set_expiration(int expiration) {

        EP_STAT estat;
        estat = Gdp21Library.INSTANCE.gdp_create_info_set_expiration(
                                    this.gdp_create_info_ptr, expiration);
    }
    
    /**
     * Set creation service name
     * @param x_service_name
     */
    public void set_creation_service(String x_service_name) {

        EP_STAT estat;
        estat = Gdp21Library.INSTANCE.gdp_create_info_set_creation_service(
                                    this.gdp_create_info_ptr, x_service_name);
    }


    //////////// Create new keypair /////////////////


    /**
     * Create new owner keypair (in memory, not on disk)
     * @param dig_alg_name Digest algorithm
     * @param key_alg_name Signing algorithm
     * @param int bits
     * @param curve_name
     * @param key_enc_alg_name on-disk crypto alg
     */
    public void new_owner_key(String dig_alg_name, String key_alg_name,
                            int bits, String curve_name, String key_enc_alg_name) {

        EP_STAT estat;
        estat = Gdp21Library.INSTANCE.gdp_create_info_new_owner_key(
                                    this.gdp_create_info_ptr, dig_alg_name, key_alg_name,
                                    bits, curve_name, key_enc_alg_name);
    }

    /**
     * Create new writer keypair (in memory, not on disk)
     * @param dig_alg_name Digest algorithm
     * @param key_alg_name Signing algorithm
     * @param int bits
     * @param curve_name
     * @param key_enc_alg_name on-disk crypto alg
     */
    public void new_writer_key(String dig_alg_name, String key_alg_name,
                            int bits, String curve_name, String key_enc_alg_name) {

        EP_STAT estat;
        estat = Gdp21Library.INSTANCE.gdp_create_info_new_writer_key(
                                    this.gdp_create_info_ptr, dig_alg_name, key_alg_name,
                                    bits, curve_name, key_enc_alg_name);
    }


    /////////////// Set already existing keypair /////////////////

    /**
     * Set already existing keypair for owner; see EP_CRYPTO_KEY.java as well
     * @param key EP_CRYPTO_KEY structure
     * @param dig_alg_name
     */
    public void set_owner_key(EP_CRYPTO_KEY key, String dig_alg_name) {
        EP_STAT estat;
        estat = Gdp21Library.INSTANCE.gdp_create_info_set_owner_key(
                                    this.gdp_create_info_ptr,
                                    key.key_ptr, dig_alg_name);
    }


    /**
     * Set already existing keypair for writer; see EP_CRYPTO_KEY.java as well
     * @param key EP_CRYPTO_KEY structure
     * @param dig_alg_name
     */
    public void set_writer_key(EP_CRYPTO_KEY key, String dig_alg_name) {
        EP_STAT estat;
        estat = Gdp21Library.INSTANCE.gdp_create_info_set_writer_key(
                                    this.gdp_create_info_ptr,
                                    key.key_ptr, dig_alg_name);
    }

    /////////////// Save stored keypairs to disk /////////////////
    /**
     * save stored keypairs (both admin and writer) to disk
     * @param key_ctx
     * @param key_enc_alg_name
     * @param key_enc_password
     */
    public void save_keys(int key_ctx, String key_enc_alg_name,
                                                String key_enc_password) {
        EP_STAT estat;
        estat = Gdp21Library.INSTANCE.gdp_create_info_save_keys(
                                    this.gdp_create_info_ptr, key_ctx, 
                                    key_enc_alg_name, key_enc_password);
    }


    //////////// arbitrary user metadata ///////////////////
    /**
     * Add arbitrary user metadata (escape hatch for extensions)
     * @param md_name
     * @param md_val
     */
    public void add_metadata(int md_name, byte[] md_val) {

        // FIXME: figure out if we need to have a +1 for len
        NativeSize md_len = new NativeSize(md_val.length);

        // Create a C string that is a copy (?) of the Java String.
        Memory memory = new Memory(md_val.length+1);

        // FIXME: not sure about alignment.
        Memory alignedMemory = memory.align(4);
        memory.clear();
        Pointer pointer = alignedMemory.share(0);
        for (int i=0; i<md_val.length; i++) {
            pointer.setByte(i, md_val[i]);
        }

        Gdp21Library.INSTANCE.gdp_create_info_add_metadata(
                            this.gdp_create_info_ptr, md_name,
                            md_len, pointer);

    }

    ////////////////////////////////////////////////////////////

    // Pointer to the C structure
    public PointerByReference gdp_create_info_ptr = null;
    
    // To keep track of whether we need to call free in destructor
    private boolean did_i_create_it = false;


}
