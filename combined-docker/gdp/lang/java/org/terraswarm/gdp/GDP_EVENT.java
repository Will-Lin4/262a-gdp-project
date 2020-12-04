/* Global Data Plane events.

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
 * Global Data Plane events.
 * 
 * @author Nitesh Mor
 */

public class GDP_EVENT {

    /**
     * Pass a pointer to an already allocated gdp_event_t C structure.
     * @param d A pointer to an existing gdp_event_t
     */
    public GDP_EVENT(PointerByReference d) {
        this.gdp_event_ptr = d;
        this.did_i_create_it = false;
        return;
    }

    /// XXX This ought to be called explicitly for all events. 
    /**
     * In case we allocated memory for the C gdp_event_t ourselves, free it.
     */
    public void finalize() {        
        if (this.did_i_create_it) {
            Gdp21Library.INSTANCE.gdp_event_free(this.gdp_event_ptr);
        }
    }

    /////////////////////////////////////////////////////////////

    public int gettype() {
        return Gdp21Library.INSTANCE.gdp_event_gettype(this.gdp_event_ptr);
    }

    public EP_STAT getstat() {
        return Gdp21Library.INSTANCE.gdp_event_getstat(this.gdp_event_ptr);
    }

    public GDP_GIN getgin() {
        return new GDP_GIN(Gdp21Library.INSTANCE.gdp_event_getgin(this.gdp_event_ptr));
    }

    public GDP_DATUM getdatum() {
        return new GDP_DATUM(Gdp21Library.INSTANCE.gdp_event_getdatum(this.gdp_event_ptr));
    }

    /////////////////////////////////////////////////////////////


    // Pointer to the C structure
    public PointerByReference gdp_event_ptr = null;
    
    // To keep track of whether we need to call free in destructor
    private boolean did_i_create_it = false;


}
