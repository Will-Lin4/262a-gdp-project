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


from MISC import *


class GDP_BUF(object):
    """
    A class only for internal use. It is only a partially
    implemented interface for the moment.
    """

    class gdp_buf_t(Structure):
        pass

    def __init__(self, **kwargs):
        """
        Creates a new buffer structure by calling C functions, unless
        an existing pointer to a C-structure is provided to us
        externally (by using a 'ptr=x' argument).
        """
        if len(kwargs) == 0:
            __func = gdp.gdp_buf_new
            __func.argtypes = []
            __func.restype = POINTER(self.gdp_buf_t)
            self.ptr = __func()
            self.did_i_create_it = True
        else:
            if "ptr" in kwargs:
                self.ptr = kwargs["ptr"]
                self.did_i_create_it = False
            else:
                raise Exception

    def __del__(self):
        """Free the memory, but only if we allocated it."""
        if self.did_i_create_it:
            __func = gdp.gdp_buf_free
            __func.argtypes = [POINTER(self.gdp_buf_t)]
            __func(self.ptr)


    def reset(self):
        """Reset the buffer"""
        __func = gdp.gdp_buf_reset
        __func.argtypes = [POINTER(self.gdp_buf_t)]
        __func.restype = c_int

        ret = __func(self.ptr)
        return int(ret)


    def getlength(self):
        "Returns the length of the data associated with this buffer"

        __func = gdp.gdp_buf_getlength
        __func.argtypes = [POINTER(self.gdp_buf_t)]
        __func.restype = c_size_t

        ret = __func(self.ptr)
        return ret


    def read(self):
        "Read data from buffer (effectively drains it too)"

        __func = gdp.gdp_buf_read
        __func.argtypes = [POINTER(self.gdp_buf_t), c_void_p, c_size_t]
        __func.restype = c_size_t

        dlen = self.getlength()
        tmp_buf = create_string_buffer(dlen)
        read_bytes = __func(self.ptr, byref(tmp_buf), dlen)

        return string_at(tmp_buf, read_bytes)


    def peek(self):
        "Peek into a buffer (does not drain it)"

        __func = gdp.gdp_buf_peek
        __func.argtypes = [POINTER(self.gdp_buf_t), c_void_p, c_size_t]
        __func.restype = c_size_t

        dlen = self.getlength()
        tmp_buf = create_string_buffer(dlen)
        peeked_bytes = __func(self.ptr, byref(tmp_buf), dlen)

        return string_at(tmp_buf, peeked_bytes)


    def drain(self):
        "simply remove the data from the buffer"

        __func = gdp.gdp_buf_drain
        __func.argtypes = [POINTER(self.gdp_buf_t), c_size_t]
        __func.restype = c_int

        dlen = self.getlength()
        ret = __func(self.ptr, dlen)

        return int(ret)


    def write(self, data):
        "write data to buffer"

        __func = gdp.gdp_buf_write
        __func.argtypes = [POINTER(self.gdp_buf_t), c_void_p, c_size_t]

        size = c_size_t(len(data))
        tmp_buf = create_string_buffer(data, len(data))
        __func(self.ptr, byref(tmp_buf), size)
