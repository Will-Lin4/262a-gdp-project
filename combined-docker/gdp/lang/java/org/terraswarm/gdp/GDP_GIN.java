/* A Global Data Plane (GDP) GIN

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
import java.util.HashMap;
import java.util.List;
import java.awt.PointerInfo;
import java.lang.Exception;


import com.sun.jna.Native;
import com.sun.jna.Memory;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.PointerByReference;
import org.terraswarm.gdp.NativeSize; // Fixed by cxh in makefile.

/**
 * A Global Data Plane (GDP) GIN
 *
 * <p>This Java wrapper may not have the latest features in the
 * underlying C implementation.</p>
 * 
 * @author Nitesh Mor, Christopher Brooks
 */
public class GDP_GIN {
    
    // The underscores in the names of public methods and variables should
    // be preserved so that they match the underlying C-based gdp code.

    /** 
     * Initialize a new Global Data Plane GIN
     *
     * @param name   Name of the log, which will be created if necessary.
     * @param iomode Should this be opened read only, read-append,
     *               append-only.  See {@link #GDP_MODE}.
     * @param info   GDP_OPEN_INFO associated with this particular in-memory instance
     * @exception    GDPException If a GDP C function returns a code great than or
     *               equal to Gdp21Library.EP_STAT_SEV_WARN.
     */
    public GDP_GIN(GDP_NAME name, GDP_MODE iomode, GDP_OPEN_INFO info) throws GDPException {

        System.out.println("GDP_GIN.java: GDP_GIN(" + name +
                           "(" + new String(name.printable_name()) +
                           "), " + iomode);
        EP_STAT estat;
        
        PointerByReference _gdp_gin_ptr = new PointerByReference();
        this.iomode = iomode;
        this.gclName = name.internal_name();
        this.open_info = info;
        
        // Open the GCL.
        // GDP.dbg_set("*=20");

        estat = Gdp21Library.INSTANCE.gdp_gin_open(ByteBuffer.wrap(this.gclName), 
                                iomode.ordinal(), info.gdp_open_info_ptr, 
                                _gdp_gin_ptr);

        // check return status; throw exception if need be.
        GDP.check_EP_STAT(estat, "gdp_gin_open(" +
                          gclName + "(" + new String(name.printable_name(),0)
                          + "), " + iomode.ordinal() + ", null, " +
                          this.gdp_gin_ptr + ") failed.");

        // Associate the C pointer to this object.
        this.gdp_gin_ptr = _gdp_gin_ptr;
        this.ginh = this.gdp_gin_ptr.getValue();
        assert this.ginh != null;

        // Add ourselves to the global map of pointers=>objects 
        // XXX Not sure if this is needed anymore with v2 API
        _allGclhs.put(this.ginh, this);
    }


    public GDP_GIN(GDP_NAME name, GDP_MODE iomode) throws GDPException {
        this(name, iomode, new GDP_OPEN_INFO());
    } 

    public GDP_GIN(GDP_NAME name) throws GDPException {
        this(name, GDP_GIN.GDP_MODE.RO); 
    }

    public GDP_GIN(PointerByReference d) {

        this.gdp_gin_ptr = d;
        this.ginh = this.gdp_gin_ptr.getValue();
    }

    /** Remove the gcl from the global list and 
     *  free the associated pointer.
     *  Note that finalize() is only called when the 
     *  garbage collector decides that there are no
     *  references to an object.  There is no guarantee 
     *  that an object will be gc'd and that finalize
     *  will be called.
     */
     public void finalize() {
        __close();
    }


    ///////////////////////////////////////////////////////////////////
    ////                   public fields                           ////
    ///////////////////////////////////////////////////////////////////

    /**
     * The internal 256 bit name for the log.
     */
    public byte[] gclName;

    /**
     * A pointer to the C gdp_gin_t structure
     */
    public Pointer ginh = null;
    public PointerByReference gdp_gin_ptr = null;

    /**
     * The I/O mode for this log
     */
    public GDP_MODE iomode;

    /**
     * Open info associated wit this GIN
     */
    public GDP_OPEN_INFO open_info;

    /**
     * I/O mode for a log.
     * <ul>
     * <li> ANY: for internal use only </li>
     * <li> RO : Read only </li>
     * <li> AO : Append only </li>
     * <li> RA : Read + append </li>
     * </ul> 
     */
    public enum GDP_MODE {
        ANY, RO, AO, RA
    }

    private boolean did_i_create_this = false;

    ///////////////////////////////////////////////////////////////////////
    //////   Creation, deletion ahead (Static methods). Be careful.  //////
    ///////////////////////////////////////////////////////////////////////

    /**
     * Create a GCL; requires an appropriate GDP_CREATE_INFO structure.
     * 
     * @param gci a GDP_CREATE_INFO structure; see {@link #GDP_CREATE_INFO}.
     * @param external_name   External name of the log to be created
     * @exception GDPException If a GDP C function returns a code great than or
     *            equal to Gdp21Library.EP_STAT_SEV_WARN.
     */
    public static void create(GDP_CREATE_INFO gci, String external_name) throws GDPException {

        EP_STAT estat;
        
        PointerByReference tmp = new PointerByReference();
        tmp.setValue(Pointer.NULL);
        estat = Gdp21Library.INSTANCE.gdp_gin_create(
                                gci.gdp_create_info_ptr,
                                external_name, tmp);

        GDP.check_EP_STAT(estat, "Creation of " + external_name + " failed.");
    }
    
    ///////////////////////////////////////////////////////////////////
    ////           public methods: append and friends              ////
    ///////////////////////////////////////////////////////////////////

    /**
     * Append data to a log. This will create a new record in the log.
     * 
     * @param datum A filled GDP_DATUM that is to be appended.
     * @param prevhash Hash of previous record
     * @exception GDPException If a GDP C function returns a code great than or
     *            equal to Gdp21Library.EP_STAT_SEV_WARN.
     */
    public void append(GDP_DATUM datum, GDP_HASH prevhash) throws GDPException {

        EP_STAT estat;
        _checkGclh(this.ginh);
        if (prevhash == null) {
            estat = Gdp21Library.INSTANCE.gdp_gin_append(this.ginh,
                                datum.gdp_datum_ptr, null);
        } else {
            estat = Gdp21Library.INSTANCE.gdp_gin_append(this.ginh,
                                datum.gdp_datum_ptr, prevhash.gdp_hash_ptr.getValue());
        }
        GDP.check_EP_STAT(estat, "gdp_gin_append(" + this.ginh +
                         ", " + datum.gdp_datum_ptr + ") failed.");
        return;
    }

    /**
     * Append data to a log, asynchronously. This will create a new record
     * in the log.
     * 
     * @param datums A List of GDP_DATUM's that ought to be appended in an
     *               asynchronous way
     * @param prevhash Hash of the previous record
     * @exception GDPException If a GDP C function returns a code great than or
     *            equal to Gdp21Library.EP_STAT_SEV_WARN.
     */
    public void append_async(List<GDP_DATUM> datums, GDP_HASH prevhash)
                                            throws GDPException {

        EP_STAT estat;
        _checkGclh(this.ginh);
        // FIXME for the moment, we only support one datum. A more general
        // memory management is yet to come.
        if (datums.size() > 1) {
            throw new GDPException("Only single datum supported at the moment.");
        }

        PointerByReference datums_ptr = new PointerByReference(
                                            datums.get(0).gdp_datum_ptr.getValue());
        estat = Gdp21Library.INSTANCE.gdp_gin_append_async(
                                this.ginh, 1, datums_ptr, null, null, null);
        GDP.check_EP_STAT(estat, "gdp_gin_append_async(" + this.ginh +
                            ", " + datums.get(0).gdp_datum_ptr + "null, null) failed.");
    }


    ///////////////////////////////////////////////////////////////////
    ////             public methods: read and friends              ////
    ///////////////////////////////////////////////////////////////////


    /**
     * Read a record by record number.
     * 
     * @param recno Record number to be read
     * @return A GDP_DATUM containing the result
     * @exception GDPException If a GDP C function returns a code great than or
     *            equal to Gdp21Library.EP_STAT_SEV_WARN.
     */
    public GDP_DATUM read_by_recno(long recno) throws GDPException {

        EP_STAT estat;
        _checkGclh(this.ginh);

        GDP_DATUM datum = new GDP_DATUM();
        estat = Gdp21Library.INSTANCE.gdp_gin_read_by_recno(
                                this.ginh, recno, datum.gdp_datum_ptr);
        GDP.check_EP_STAT(estat, "gdp_gin_read() failed.");

        return datum;
    }

    /** 
     * Asynchronous version of read
     * only. Not for any future records
     * See the documentation in C library for examples.
     * 
     * @param firstrec  The record num to start reading from
     * @param nrecs   Max number of records to be returned. 
     * @exception GDPException If a GDP C function returns a code great than or
     *            equal to Gdp21Library.EP_STAT_SEV_WARN.
     */
    public void read_by_recno_async(long firstrec, int nrecs)
                                                throws GDPException {

        EP_STAT estat;
        _checkGclh(this.ginh);

        estat = Gdp21Library.INSTANCE.gdp_gin_read_by_recno_async(
                            this.ginh, firstrec, nrecs, null, null);

        GDP.check_EP_STAT(estat, "gdp_gin_read_by_recno_async() failed.");
    }

    ///////////////////////////////////////////////////////////////////
    ////         public methods: subscriptions and friends         ////
    ///////////////////////////////////////////////////////////////////


    /** 
     * Start a subscription to a log.
     * See the documentation in C library for examples.
     * 
     * @param start  The record num to start the subscription from
     * @param nrecs   Max number of records to be returned. 0 => infinite
     * @exception GDPException If a GDP C function returns a code great than or
     *            equal to Gdp21Library.EP_STAT_SEV_WARN.
     */
    public void subscribe_by_recno(long start, int nrecs) throws GDPException {

        EP_STAT estat;
        _checkGclh(this.ginh);
        estat = Gdp21Library.INSTANCE.gdp_gin_subscribe_by_recno(
                                    this.ginh, start, nrecs,
                                        null, null, null);

        GDP.check_EP_STAT(estat, "gdp_gin_subscribe_by_recno() failed.");
    }


    public void unsubscribe() throws GDPException {
        EP_STAT estat;
        _checkGclh(this.ginh);
        estat = Gdp21Library.INSTANCE.gdp_gin_unsubscribe(this.ginh, null, null);
        GDP.check_EP_STAT(estat, "gdp_gin_unsubscribe() failed.");
    }


    /** 
     * Get the next event.
     * 
     * @param timeout_msec  Time (in ms) for which to block. Can be used 
     *                      to block eternally as well.
     */
    public static GDP_EVENT get_next_event(GDP_GIN obj, int timeout_msec) {
        // This method is used by the Ptolemy interface to the GDP.
        EP_TIME_SPEC timeout_spec = new EP_TIME_SPEC(timeout_msec/1000,
                0, /* nanoseconds */
                0.001f /* accuracy in seconds */);
        return get_next_event(obj, timeout_spec);
    }

    /** 
     * Get data from next record for a subscription or multiread
     * This is a wrapper around 'get_next_event', and works only 
     * for subscriptions, multireads.
     * 
     * @param timeout_msec  Time (in ms) for which to block. Can be used 
     *                      to block eternally as well.
     */
    public static GDP_EVENT get_next_event(GDP_GIN obj, EP_TIME_SPEC timeout) {
        if (obj == null) {
            return _helper_get_next_event(null, timeout);
        } else {
            _checkGclh(obj.ginh);
            return _helper_get_next_event(obj.ginh, timeout);
        }
    }
    
    ///////////////////////////////////////////////////////////////////
    ////                   private methods                         ////

    
    public static GDP_EVENT _helper_get_next_event(Pointer ginh, EP_TIME_SPEC timeout) {
        
        // Get the event pointer. ginh can be null.
        PointerByReference gdp_event_ptr = Gdp21Library.INSTANCE.gdp_event_next(ginh, timeout);
        return new GDP_EVENT(gdp_event_ptr);
    }
    


    ///////////////////////////////////////////////////////////////////
    ////                   private methods                         ////

    /** If ginh is null, then throw an exception stating that close()
     *  was probably called.
     */   
    private static void _checkGclh(Pointer ginh) {
        if (ginh == null) {
            throw new NullPointerException("The pointer to the C gdp_gin_t structure was null."
                                           + "Perhaps close() was called?  "
                                           + "See https://gdp.cs.berkeley.edu/redmine/issues/83");
        }
    }

    /** Close the GCL.
     */
    private void __close() {
        // If close() is called twice, then the C code aborts the process.
        // See https://gdp.cs.berkeley.edu/redmine/issues/83

        //  Added synchronization, see https://gdp.cs.berkeley.edu/redmine/issues/107
        synchronized (this.ginh) {
            if (this.ginh != null) {
                // Remove ourselves from the global list.
                _allGclhs.remove(this.ginh);
                
                // Free the associated gdp_gin_t.
                Gdp21Library.INSTANCE.gdp_gin_close(this.ginh);
                this.ginh = null;
            }
        }
    }


    ///////////////////////////////////////////////////////////////////
    ////                   private variables                       ////

    /**
     * A global list of objects, which is useful for get_next_event().
     */
    private static HashMap<Pointer, Object> _allGclhs = 
            new HashMap<Pointer, Object>();

}
