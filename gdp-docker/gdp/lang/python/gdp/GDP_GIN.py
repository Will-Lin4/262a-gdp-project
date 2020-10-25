#!/usr/bin/env python

# ----- BEGIN LICENSE BLOCK -----
#	GDP: Global Data Plane
#	From the Ubiquitous Swarm Lab, 490 Cory Hall, U.C. Berkeley.
#
#	Copyright (c) 2015-2019, Regents of the University of California.
#	All rights reserved.
#
#	Permission is hereby granted, without written agreement and without
#	license or royalty fees, to use, copy, modify, and distribute this
#	software and its documentation for any purpose, provided that the above
#	copyright notice and the following two paragraphs appear in all copies
#	of this software.
#
#	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
#	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
#	PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
#	EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
#	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#	FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION,
#	IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO
#	OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS,
#	OR MODIFICATIONS.
# ----- END LICENSE BLOCK -----


import pprint
from MISC import *
from GDP_HASH import *
from GDP_SIG import *
from GDP_EVENT import *
from GDP_NAME import *
from GDP_DATUM import *
from GDP_MD import *
from GDP_OPEN_INFO import *


class GDP_GIN(object):

    """
    A class that represents a GDP Instance. A GIN resembles an open
    file handle in various ways.

    Note that get_next_event is both a class method (for events on any of
    the GIN's) as well as an instance method (for events on a particular
    GIN). The auto-generated documentation might not show this, but
    something to keep in mind.
    """

    class gdp_gin_t(Structure):
        "Corresponds to gdp_gin_t structure exported by C library"
        pass


    # XXX: Check if this works. More details here:
    # http://python.net/crew/theller/ctypes/tutorial.html#callback-functions
    # The first argument, I believe, is the return type, which I think is void*
    gdp_gin_sub_cbfunc_t = CFUNCTYPE(c_void_p, POINTER(gdp_gin_t),
                                     POINTER(GDP_DATUM.gdp_datum_t), c_void_p)


    def __init__(self, name, iomode, open_info={}, **kwargs):
        """
        Open a GDP instance for a GDP object with given name and io-mode.

        name=<name-of-gin>, iomode=<mode>
        name is a GDP_NAME object for the GDP object
        mode is one of the following: GDP_MODE_ANY, GDP_MODE_RO,
                                      GDP_MODE_AO, GDP_MODE_RA
        open_info is a dictionary. It contains extra information
        such as the private signature key, etc. The key may be
        optional, depending on how the log was created and if a
        log-server is set up to reject unsigned appends, for example.
        """

        if len(kwargs)==0:

            # __ptr is just a C style pointer, that we will assign to
            # something
            __ptr = POINTER(self.gdp_gin_t)()

            # we do need an internal represenation of the name.
            _name_python = name.internal_name()
            # convert this to a string that ctypes understands.
            # Some ctypes magic ahead
            buf = create_string_buffer(_name_python, 32+1)
            _name_ctypes_ptr = cast(byref(buf), POINTER(GDP_NAME.name_t))
            _name_ctypes = _name_ctypes_ptr.contents

            # (optional) Do a quick sanity checking on open_info
            #   use it to get a GDP_OPEN_INFO structure
            __gdp_open_info = GDP_OPEN_INFO(open_info)

            # open an existing gin
            __func = gdp.gdp_gin_open
            __func.argtypes = [GDP_NAME.name_t, c_int,
                               POINTER(GDP_OPEN_INFO.gdp_open_info_t),
                               POINTER(POINTER(self.gdp_gin_t))]
            __func.restype = EP_STAT

            estat = __func(_name_ctypes, iomode, __gdp_open_info.ptr,
                                pointer(__ptr))
            check_EP_STAT(estat)

            self.ptr = __ptr
            ## Create the instance method 'get_next_event', in addition to
            ## already existing class method
            self.get_next_event = self.__get_next_event
            self.did_i_create_it = True

        else:

            if "ptr" in kwargs:
                self.ptr = kwargs["ptr"]
                self.get_next_event = self.__get_next_event
                self.did_i_create_it = False
            else:
                raise Exception


    def __del__(self):
        "close this GCL handle, and free the allocated resources"

        assert self.ptr is not False    # null pointers have false bool val

        if self.did_i_create_it:
            # call the C function to free associated C memory block
            __func = gdp.gdp_gin_close
            __func.argtypes = [POINTER(self.gdp_gin_t)]
            __func.restype = EP_STAT

            estat = __func(self.ptr)
            check_EP_STAT(estat)

    def __eq__(self, other):
        ## XXX is this correct?
        return addressof(self.ptr.contents) == addressof(other.ptr.contents)

    @classmethod
    def create(cls, name, logd_name, metadata):
        """
        create a new GCL with 'name' on 'logd_name'
        metadata is a dictionary with keys as 32-bit unsigned integers
        """

        # we do need an internal represenation of the names.
        _name_python = name.internal_name()
        logd_name_python = logd_name.internal_name()

        # convert this to a string that ctypes understands. Some ctypes magic
        # ahead
        buf1 = create_string_buffer(_name_python, 32+1)
        _name_ctypes_ptr = cast(byref(buf1), POINTER(GDP_NAME.name_t))
        _name_ctypes = _name_ctypes_ptr.contents

        buf2 = create_string_buffer(logd_name_python, 32+1)
        logd_name_ctypes_ptr = cast(byref(buf2), POINTER(GDP_NAME.name_t))
        logd_name_ctypes = logd_name_ctypes_ptr.contents

        throwaway_ptr = POINTER(cls.gdp_gin_t)()

        md = GDP_MD()
        for k in metadata:
            md.add(k, metadata[k])

        __func = gdp.gdp_gin_create
        __func.argtypes = [GDP_NAME.name_t, GDP_NAME.name_t,
                                POINTER(GDP_MD.gdp_md_t),
                                POINTER(POINTER(cls.gdp_gin_t))]
        __func.restype = EP_STAT

        estat = __func(_name_ctypes, logd_name_ctypes,
                            md.ptr, throwaway_ptr)
        check_EP_STAT(estat)

        # close this gin/gob to prevent side-effects
        __func = gdp.gdp_gin_close
        __func.argtypes = [POINTER(cls.gdp_gin_t)]
        __func.restype = EP_STAT

        estat = __func(throwaway_ptr)
        check_EP_STAT(estat)
        return


    def getmetadata(self):
        """
        return the metadata included at creation time.
            Represented as a python dict
        """

        __func = gdp.gdp_gin_getmetadata
        __func.argtypes = [POINTER(self.gdp_gin_t),
                                POINTER(POINTER(GDP_MD.gdp_md_t))]
        __func.restype = EP_STAT

        gmd = POINTER(GDP_MD.gdp_md_t)()
        estat = __func(self.ptr, byref(gmd))
        check_EP_STAT(estat)

        md = GDP_MD(ptr=gmd)
        return md


    def getname(self):
        "Get the name of the corresponding GOB, returns a GDP_NAME object"

        __func = gdp.gdp_gin_getname
        __func.argtypes = [POINTER(self.gdp_gin_t)]
        __func.restype = POINTER(GDP_NAME.name_t)

        _name_pointer = __func(self.ptr)
        _name = string_at(_name_pointer, 32)
        return GDP_NAME(_name)


    def getnrecs(self):
        "Get number of records"

        __func = gdp.gdp_gin_getnrecs
        __func.argtypes = [POINTER(self.gdp_gin_t)]
        __func.restype = gdp_recno_t

        ret = __func(self.ptr)
        return ret


    def print_to_file(self, fh):
        """
        Print this GDP object to a file. Could be sys.stdout
        The actual printing is done by the C library
        """
        __fh = PyFile_AsFile(fh)
        __func = gdp.gdp_gin_print
        __func.argtypes = [POINTER(self.gdp_gin_t), FILE_P]
        # ignore the return type
        __func(self.ptr, __fh)



    ##################################################################
    ######## Various read functions (sync/async|ts/recno/hash) #######
    ##################################################################

    def __read(self, query_param):
        """
        An internal helper function for synchronous read. Either read
        by record number, timestamp, or the hash.

        If query_param is 'int', we assume it is query by record number.
        If query_param is 'dict', we assume it is query by timestamp.
        If query_param is 'str', we assume it is query by hash.
        """

        if isinstance(query_param, int):
            __query_param = gdp_recno_t(query_param)

            __func = gdp.gdp_gin_read_by_recno
            __func.argtypes = [POINTER(self.gdp_gin_t), gdp_recno_t,
                                    POINTER(GDP_DATUM.gdp_datum_t)]
            __func.restype = EP_STAT

        elif isinstance(query_param, str):

            # TODO This is read by hash. We will come back to it
            # later when the C-library implements this functionality.
            raise NotImplementedError

        elif isinstance(query_param, dict):

            raise NotImplementedError

            # TODO check whether the following commented out version
            # works when the C-library implements this functionality.

            # __query_param = EP_TIME_SPEC()
            # __query_param.tv_sec = c_int64(query_param['tv_sec'])
            # __query_param.tv_nsec = c_uint32(query_param['tv_nsec'])
            # __query_param.tv_accuracy = c_float(query_param['tv_accuracy'])

            # __func = gdp.gdp_gin_read_by_ts
            # __func.argtypes = [POINTER(self.gdp_gin_t),
            #                         POINTER(EP_TIME_SPEC),
            #                         POINTER(GDP_DATUM.gdp_datum_t)]
            # __func.restype = EP_STAT

        else:
            # should never reach here
            assert False

        datum = GDP_DATUM()
        estat = __func(self.ptr, __query_param, datum.ptr)
        check_EP_STAT(estat)

        return datum


    def __read_async(self, start, numrecs, cbfunc, cbarg):
        """
        same as __read, except that this is the async version (and
        enables querying multiple records at once).

        Additionally, this does not return a GDP_DATUM; instead,
        the user is supposed to ask for events (see `get_next_event`).

        For now, callback functionality is not very well tested.
        """

        if isinstance(start, int):
            # casting start to ctypes
            __start = gdp_recno_t(start)
            __start_type = gdp_recno_t
            __func = gdp.gdp_gin_read_by_recno_async

        elif isinstance(start, str):
            ## TODO: this is query by hash. The underlying
            ## C-library does not implement this as of now.
            raise NotImplementedError

        elif isinstance(start, dict):

            raise NotImplementedError

            ## TODO: read by timestamp. Verify that the following
            ## commented out version works when the underlying
            ## C-library implements this functionality.

            # __start = EP_TIME_SPEC()
            # __start.tv_sec = c_int64(start['tv_sec'])
            # __start.tv_nsec = c_uint32(start['tv_nsec'])
            # __start.tv_accuracy = c_float(start['tv_accuracy'])
            # __start_type = POINTER(EP_TIME_SPEC)
            # __func = gdp.gdp_gin_read_by_ts_async

        else:
            # should never reach here.
            assert False

        # casting numrecs to ctypes
        __numrecs = c_int32(numrecs)

        # casting the python function to the callback function
        if cbfunc == None:
            __cbfunc = None
        else:
            __cbfunc = self.gdp_gin_sub_cbfunc_t(cbfunc)

        if cbfunc == None:
            __func.argtypes = [POINTER(self.gdp_gin_t), __start_type,
                                c_int32, c_void_p, c_void_p]
        else:
            __func.argtypes = [POINTER(self.gdp_gin_t), __start_type,
                                c_int32, self.gdp_gin_sub_cbfunc_t, c_void_p]
        __func.restype = EP_STAT

        estat = __func(self.ptr, __start, __numrecs, __cbfunc, cbarg)
        check_EP_STAT(estat)
        return estat


    def read_by_recno(self, recno):
        """Returns a GDP_DATUM object corresponding to specified recno"""
        return self.__read(recno)


    def read_by_hash(self, hashbytes):
        """Same as read_by_recno, except takes a hash instead of recno"""
        return self.__read(hashbytes)


    def read_by_ts(self, tsdict):
        """
        Same as 'read_by_recno', but takes a time-stamp dictionary
        instead of a record number.
        The time-stamp dictionary has the following fields:
        - tv_sec: seconds since epoch (an integer)
        - tv_nsec: nano seconds (an integer)
        - tv_accuracy: accuracy (a float)
        """
        return self.__read(tsdict)


    def read_by_recno_async(self, start, numrecs):
        """
        Asynchronous version of `read_by_recno` that supports reading
        multiple records at a time.
        For now, callbacks are not exposed to end-user. Events are
        generated instead.
        """
        return self.__read_async(start, numrecs, None, None)


    def read_by_hash_async(self, starthash, numrecs):
        """
        Asynchronous version of `read_by_hash` that supports reading
        multiple records at a time.
        For now, callbacks are not exposed to end-user. Events are
        generated instead.
        """
        return self.__read_async(starthash, numrecs, None, None)


    def read_by_ts_async(self, startdict, numrecs):
        """
        Asynchronous version of `read_by_ts` that supports reading
        multiple records at a time.
        For now, callbacks are not exposed to end-user. Events are
        generated instead.
        """
        return self.__read_async(startdict, numrecs, None, None)


    ##################################################################
    ########### Various subscribe functions (ts/recno/hash) ##########
    ##################################################################

    def __subscribe(self, start, numrecs, timeout, cbfunc, cbarg):
        """
        This works somewhat similar to the subscribe in GDP C api.
        Callback function support is experimental. Please use the
        events interface if possible.

        If `start` is 'int', we assume it is subscription by record number.
        If `start` is 'dict', we assume it is subscription by timestamp.
        If `start` is 'str', we assume it is subscription by hash.
        """

        if isinstance(start, int):
            # casting start to ctypes
            __start = gdp_recno_t(start)

            __start_type = gdp_recno_t
            __func = gdp.gdp_gin_subscribe_by_recno

        elif isinstance(start, str):
            raise NotImplementedError

        elif isinstance(start, dict):
            raise NotImplementedError

            # __start = EP_TIME_SPEC()
            # __start.tv_sec = c_int64(start['tv_sec'])
            # __start.tv_nsec = c_uint32(start['tv_nsec'])
            # __start.tv_accuracy = c_float(start['tv_accuracy'])

            # __start_type = POINTER(EP_TIME_SPEC)
            # __func = gdp.gdp_gin_subscribe_ts

        else:
            # should never reach here
            assert False

        # casting numrecs to ctypes
        __numrecs = c_int32(numrecs)

        # if timeout is None, then we just skip this
        if timeout == None:
            __timeout = None
        else:
            __timeout = EP_TIME_SPEC()
            __timeout.tv_sec = c_int64(timeout['tv_sec'])
            __timeout.tv_nsec = c_uint32(timeout['tv_nsec'])
            __timeout.tv_accuracy = c_float(timeout['tv_accuracy'])

        # casting the python function to the callback function
        if cbfunc == None:
            __cbfunc = None
        else:
            __cbfunc = self.gdp_gin_sub_cbfunc_t(cbfunc)

        if cbfunc == None:
            __func.argtypes = [POINTER(self.gdp_gin_t), __start_type,
                                c_int32, POINTER(EP_TIME_SPEC),
                                c_void_p, c_void_p]
        else:
            __func.argtypes = [POINTER(self.gdp_gin_t), __start_type,
                                c_int32, POINTER(EP_TIME_SPEC),
                                self.gdp_gin_sub_cbfunc_t, c_void_p]

        __func.restype = EP_STAT

        estat = __func(
            self.ptr, __start, __numrecs, __timeout, __cbfunc, cbarg)
        check_EP_STAT(estat)
        return estat


    def subscribe_by_recno(self, start, numrecs, timeout):
        """
        Subscriptions by a record number.
        Refer to the C-API for more details.
        For now, callbacks are not exposed to end-user. Events are
        generated instead.
        """
        return self.__subscribe(start, numrecs, timeout, None, None)


    def subscribe_by_hash(self, starthash, numrecs, timeout):
        """Subscriptions, but by a hash instead of a record number"""
        return self.__subscribe(starthash, numrecs, timeout, None, None)


    def subscribe_by_ts(self, startdict, numrecs, timeout):
        """Subscriptions, but by a timestamp dictionary instead of recno"""
        return self.__subscribe(startdict, numrecs, timeout, None, None)


    def unsubscribe(self, cbfunc=None, cbarg=None):
        """ Terminates existing subscription.  """

        # casting the python function to the callback function
        if cbfunc == None:
            __cbfunc = None
        else:
            __cbfunc = self.gdp_gin_sub_cbfunc_t(cbfunc)

        __func = gdp.gdp_gin_unsubscribe
        if cbfunc is None:
            __func.argtypes = [POINTER(self.gdp_gin_t), c_void_p, c_void_p]
        else:
            __func.argtypes = [POINTER(self.gdp_gin_t),
                                self.gdp_gin_sub_cbfunc_t, c_void_p]
        __func.restype = EP_STAT

        estat = __func(self.ptr, __cbfunc, cbarg)
        check_EP_STAT(estat)
        return estat

    ##################################################################
    ############### Various Append functions #########################
    ##################################################################

    def append(self, datum, prevhash=None):
        """
        Write a datum to the GCL. If prevhash is not supplied, the
        C library inserts a reasonable value.
        """

        assert isinstance(datum, GDP_DATUM)
        assert isinstance(prevhash, GDP_HASH) or prevhash is None

        __prevhash_type = c_void_p if prevhash is None \
                                else POINTER(GDP_HASH.gdp_hash_t)
        __prevhash_val = None if prevhash is None else prevhash.ptr

        __func = gdp.gdp_gin_append
        __func.argtypes = [POINTER(self.gdp_gin_t),
                                POINTER(GDP_DATUM.gdp_datum_t),
                                __prevhash_type]
        __func.restype = EP_STAT

        estat = __func(self.ptr, datum.ptr, __prevhash_val)
        check_EP_STAT(estat)


    def append_async(self, datum_list, prevhash=None):
        """
        Async version of append. A writer ought to check return status
        by invoking get_next_event, potentially in a different thread.

        [This is different from append_async, which only takes a
        single datum]
        """

        assert isinstance(datum_list, list)
        assert isinstance(prevhash, GDP_HASH) or prevhash is None

        ## ctypes magic.
        ## See https://stackoverflow.com/questions/47086852/
        py_ptr_list = [x.ptr for x in datum_list]
        n = len(datum_list)
        _ptrs = (POINTER(GDP_DATUM.gdp_datum_t)*n)(*py_ptr_list)

        __prevhash_type = c_void_p if prevhash is None \
                                else POINTER(GDP_HASH.gdp_hash_t)
        __prevhash_val = None if prevhash is None else prevhash.ptr

        __func = gdp.gdp_gin_append_async
        __func.argtypes = [ POINTER(self.gdp_gin_t), c_int,
                            POINTER(POINTER(GDP_DATUM.gdp_datum_t)),
                            __prevhash_type, c_void_p, c_void_p ]
        __func.restype = EP_STAT

        estat = __func(self.ptr, c_int(n), _ptrs,
                                        __prevhash_val, None, None)
        check_EP_STAT(estat)


    ##################################################################
    ############### Various Helper functions #########################
    ##################################################################

    @classmethod
    def _helper_get_next_event(cls, __gin_handle, timeout):
        """
        Get the events for GCL __gin_handle by calling the C library.
        If __gin_handle is None, then get events for any open GCL

        Returns at most one event.
        """

        __func = gdp.gdp_event_next

        # Find the 'type' we need to pass to argtypes
        if __gin_handle == None:
            __func_arg1_type = c_void_p
        else:
            __func_arg1_type = POINTER(cls.gdp_gin_t)

        # if timeout is None, then we just skip this
        if timeout == None:
            __timeout = None
            __func_arg2_type = c_void_p
        else:
            __timeout = EP_TIME_SPEC()
            __timeout.tv_sec = c_int64(timeout['tv_sec'])
            __timeout.tv_nsec = c_uint32(timeout['tv_nsec'])
            __timeout.tv_accuracy = c_float(timeout['tv_accuracy'])
            __func_arg2_type = POINTER(EP_TIME_SPEC)

        __func.argtypes = [__func_arg1_type, __func_arg2_type]
        __func.restype = POINTER(GDP_EVENT.gdp_event_t)

        event_ptr = __func(__gin_handle, __timeout)
        if bool(event_ptr) == False:  # Null pointers have false boolean value
            return None

        return GDP_EVENT(ptr=event_ptr)


    @classmethod
    def get_next_event(cls, timeout):
        """ Get events for ANY open gin """
        return cls._helper_get_next_event(None, timeout)


    def __get_next_event(self, timeout):
        """ Get events for this particular GCL """
        event = self._helper_get_next_event(self.ptr, timeout)
        if event is not None:
            assert event["gin"] == self
        return event
