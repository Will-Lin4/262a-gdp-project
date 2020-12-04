/* Global Data Plane Utility Datum

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
 * A GDP Datum. The original C GDP Datum may create confusion since
 * data is stored in a buffer (which unsurprisingly acts like a
 * buffer).
 *
 * @author nitesh mor
 *
 */
public class GDP_DATUM {

    /**
     * Create a new datum.
     */
    public GDP_DATUM() {
        this.gdp_datum_ptr = Gdp21Library.INSTANCE.gdp_datum_new();
        this.did_i_create_it = true;
        return;
    }
    
    /**
     * Use an already allocated C data structure.
     * @param p Pointer to existing C memory of type gdp_datum_t
     */    
    public GDP_DATUM(PointerByReference d) {
        this.gdp_datum_ptr = d;
        this.did_i_create_it = false;
        return;
    }
    
    /**
     * If we allocated memory ourselves, free it.
     */
    public void finalize() { 
        if (this.did_i_create_it == true) {
            Gdp21Library.INSTANCE.gdp_datum_free(this.gdp_datum_ptr);
        }
    }
    

    public void reset() {
        Gdp21Library.INSTANCE.gdp_datum_reset(this.gdp_datum_ptr);
    }

    // TODO implement gdp_datum_copy

    // TODO implement gdp_datum_hash

    // TODO implement gdp_datum_hash_equal

    public void vrfy(GDP_GIN gin) throws GDPException{
        EP_STAT estat;
        estat = Gdp21Library.INSTANCE.gdp_datum_vrfy(
                        this.gdp_datum_ptr, gin.ginh);
        GDP.check_EP_STAT(estat, "gdp_datum_vrfy failed");
    }

    /**
     * Get the associated record number.
     * @return Record number
     */
    public long getrecno() {
        long recno = Gdp21Library.INSTANCE.gdp_datum_getrecno(
                                        this.gdp_datum_ptr);
        return recno;
    }
    
    /**
     * Get the timestamp.
     * @return timestamp associated with the C data structure
     */
    public EP_TIME_SPEC getts() {
        EP_TIME_SPEC ts = new EP_TIME_SPEC();
        Gdp21Library.INSTANCE.gdp_datum_getts(this.gdp_datum_ptr, ts);
        return ts;
    }
    
    /**
     * Get the length of data in the bufffer.
     * @return Length of buffer
     */
    public NativeSize getdlen() {
        PointerByReference buf = Gdp21Library.INSTANCE.gdp_datum_getbuf(
                                        this.gdp_datum_ptr);
        NativeSize len = Gdp21Library.INSTANCE.gdp_buf_getlength(buf);
        return len;
    }
    
    /**
     * Get the data in the buffer. Internally, queries the buffer and
     * effectively drains it as well.
     * @return the bytes in the buffer or null if the pointer to the
     * buffer is null.
     */
    public byte[] getbuf(){
        PointerByReference buf = Gdp21Library.INSTANCE.gdp_datum_getbuf(
                this.gdp_datum_ptr);
        NativeSize len = Gdp21Library.INSTANCE.gdp_buf_getlength(buf);
        Pointer bufptr = Gdp21Library.INSTANCE.gdp_buf_getptr(buf, len);
        if (bufptr == null) {
            return null;
        }
        byte[] bytes = bufptr.getByteArray(0, len.intValue());
        return bytes;
    }
    
    
    /**
     * Set the buffer to specified bytes.
     * @param data The data
     */
    public void setbuf(byte[] data){
        
        // Create a C string that is a copy (?) of the Java String.
        Memory memory = new Memory(data.length);
        // FIXME: not sure about alignment.
        Memory alignedMemory = memory.align(4);
        memory.clear();
        Pointer pointer = alignedMemory.share(0);
        
        for (int i=0; i<data.length; i++) {
            pointer.setByte(i,data[i]);
        }

        // Now feed this data into the gdp buffer
        PointerByReference dbuf = Gdp21Library.INSTANCE.gdp_datum_getbuf(
                                    this.gdp_datum_ptr);
        Gdp21Library.INSTANCE.gdp_buf_write(dbuf, pointer, 
                                new NativeSize(data.length));
    }

    // TODO implement gdp_datum_getsig

    // The associated C data structure.
    public PointerByReference gdp_datum_ptr = null;

    // To keep track of whether we need to free the associated C memory.
    private boolean did_i_create_it = false;
    

}
