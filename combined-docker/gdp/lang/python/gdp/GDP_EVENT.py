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

import gdp as _gdp
from MISC import *

class GDP_EVENT:

    """ A class that represents a GDP event """

    class gdp_event_t(Structure):
        pass

    def __init__(self, **kwargs):
        """
        Creates a new object, but associate it with an existing memory
        location.
        """
        if len(kwargs) == 0:
            raise Exception     # events are not created directly by users
        else:
            if "ptr" in kwargs:
                self.ptr = kwargs["ptr"]
            else:
                raise Exception     # FIXME

    def __del__(self):
        """
        Destructor: Does nothing if this GDP datum was created by
        passing an exisiting datum pointer
        """

        # Events are freed by the user, even when not created directly.
        # Hence, we do not need to keep track of whether we created them
        __func = gdp.gdp_event_free
        __func.argtypes = [POINTER(self.gdp_event_t)]
        __func(self.ptr)


    def print_(self, fh):
        """
        Print the GDP event C memory location contents to a file handle fh.
        fh could be sys.stdout, or any other open file handle. Flags are
        predefined constants (see GDP_EVENT_PR*)

        Note: This just calls the corresponding C library function which
        handles the printing
        """

        # need to convert this file handle to a C FILE*
        __fh = PyFile_AsFile(fh)
        __func = gdp.gdp_event_print
        __func.argtypes = [POINTER(self.gdp_event_t), FILE_P]
        # ignore the return value
        __func(self.ptr, __fh)


    def __getitem__(self, key):

        if key == "type":
            return self.gettype()
        if key == "stat":
            return self.getstat()
        if key == "gin":
            return self.getgin()
        if key == "datum":
            return self.getdatum()
        if key == "udata":
            return self.getudata()
        raise NotImplementedError


    def gettype(self):
        """ Get the event type for this event """

        __func = gdp.gdp_event_gettype
        __func.argtypes = [POINTER(self.gdp_event_t)]
        __func.restype = c_int

        ret = __func(self.ptr)
        return int(ret)


    def getstat(self):

        __func = gdp.gdp_event_getstat
        __func.argtypes = [POINTER(self.gdp_event_t)]
        __func.restype = EP_STAT

        event_ep_stat = __func(self.ptr)
        check_EP_STAT(event_ep_stat)
        return event_ep_stat


    def getgin(self):

        __func = gdp.gdp_event_getgin
        __func.argtypes = [POINTER(self.gdp_event_t)]
        __func.restype = POINTER(_gdp.GDP_GIN.gdp_gin_t)

        gin_ptr = __func(self.ptr)
        gin = _gdp.GDP_GIN(None, None, ptr=gin_ptr)
        gin.event = self # garbage collection workaround
        return gin


    def getdatum(self):

        __func = gdp.gdp_event_getdatum
        __func.argtypes = [POINTER(self.gdp_event_t)]
        __func.restype = POINTER(_gdp.GDP_DATUM.gdp_datum_t)
        datum_ptr = __func(self.ptr)
        datum = _gdp.GDP_DATUM(ptr=datum_ptr)
        datum.event = self # garbage collection workaround
        return datum

    def getudata(self):

        __func = gdp.gdp_event_getudata
        __func.argtypes = [POINTER(self.gdp_event_t)]
        __func.restype = c_void_p

        _udata = __func(self.ptr)
        udata = int(cast(_udata, c_char_p).value)

        return udata
