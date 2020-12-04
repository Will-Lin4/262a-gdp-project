/* Global Data Plane Utility functions.
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
import java.util.Arrays;
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
 * Utility functions for the GDP. For the lack of a better name and to
 * keep things concise, we just call it GDP.  Most (if not all)
 * members of this class should be static. The auto-generated
 * constants in Gdp21Library.java that should be made available to the
 * users should also ideally be copied here.
 *  
 * @author Nitesh Mor, Christopher Brooks
 */
public class GDP {

    /**
     * Initialize GDP library, with any parameters specified
     * in configuration files.
     */
    public static void gdp_init() {
        EP_STAT estat;
        estat = Gdp21Library.INSTANCE.gdp_init((Pointer)null);        
        check_EP_STAT(estat);        
       
    }
    
    /**
     * Initialize the GDP library
     * @param gdpRouter Address of GDP routers to connect to. Overrides any
     *      configuration settings.
     */
    public static void gdp_init(String gdpRouter) {
        EP_STAT estat;
        estat = Gdp21Library.INSTANCE.gdp_init(gdpRouter);
        check_EP_STAT(estat);
        
    }
    
    /** 
     * Set the debug levels for the EP/GDP C library
     * @param debug_level A string of the form X=Y, Y is an integer.
     */
    public static void dbg_set(String debug_level) {

        Gdp21Library.INSTANCE.ep_dbg_set(debug_level);
    }

    /** 
     * Error checking. For any GDP call that returns an EP_STAT 
     * structure, this is where we decode the integer return code.
     * @param estat The EP_STAT structure that contains the error code.
     */
    public static boolean check_EP_STAT(EP_STAT estat){

        int code = estat.code;

        int EP_STAT_SEVERITY = (code >>> Gdp21Library._EP_STAT_SEVSHIFT)
                & ((1 << Gdp21Library._EP_STAT_SEVBITS) - 1);

        //System.out.println("check_EP_STAT(" + estat.code + ") " + ep_stat_toStr(estat));

        return (EP_STAT_SEVERITY < Gdp21Library.EP_STAT_SEV_WARN);
    }

    /** 
     * Error checking. For any GDP call that returns an EP_STAT 
     * structure, this is where we decode the integer return code.
     * @param estat The EP_STAT structure that contains the error code.
     * @param message The message to be used if an exception is thrown.
     * @exception GDPException If {@link #check_EP_STAT(EP_STAT)} returns false.
     */
    public static void check_EP_STAT(EP_STAT estat, String message)
            throws GDPException {
        if (!check_EP_STAT(estat)) {
            throw new GDPException(message, estat);
        }
    }

    /** Given a EP_STAT, return the description of the status.
     *  @param estat The status
     *  @return The status
     */
    public static String ep_stat_toStr(EP_STAT estat) {
        int length = 200;
        ByteBuffer buffer = ByteBuffer.allocate(length);
        Pointer statusMessage = Gdp21Library.INSTANCE
                                    .ep_stat_tostr((EP_STAT.ByValue)estat,
                                                buffer, new NativeSize(length));
        return new String(buffer.array()).trim();
    }

}
