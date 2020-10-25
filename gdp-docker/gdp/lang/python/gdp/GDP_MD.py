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

# metadata id
gdp_md_id_t = c_uint32

class GDP_MD:

    """
    GCL Metadata equivalent -- for internal use only.
    The C equivalent exposed to python users is of a dictionary.

    """

    class gdp_md_t(Structure):
        pass


    def __init__(self, **kwargs):
        """ Constructor: calls the C-function to allocate memory. """

        # We need to be a bit creative here. The python object
        # can be created by allocating new memory for a
        # corresponding gdp_md_t C structure, or it could refer
        # to a gdp_md_t structure created by other means (such
        # as when querying for metadata of a GIN). In the former
        # case, it is our responsibility to clean things up when
        # we no longer need them. However, such automated cleanup
        # may cause problems for the latter case.

        if len(kwargs)==0:
            __func = gdp.gdp_md_new
            __func.argtypes = [c_int]
            __func.restype = POINTER(self.gdp_md_t)

            # Why 0? we can always keep adding entries by calling
            # gdp_md_add (sure it's a bit inefficient, but
            # how often do we need the absolute best performance
            # for gdp_md_t creation?).
            self.ptr = __func(c_int(0))
            self.did_i_create_it = True

        else:
            # The situation when python object was created by
            # passing in a pointer (i.e. memory managed by
            # someone else)
            if "ptr" in kwargs:
                self.ptr = kwargs["ptr"]
                self.did_i_create_it = False
            else:
                raise Exception


    def __del__(self):
        """ Destructor: Frees the C structure """

        if self.did_i_create_it:
            __func = gdp.gdp_md_free
            __func.argtypes = [POINTER(self.gdp_md_t)]

            __func(self.ptr)


    def dump(self, fh, detail, indent):
        """
        Print the gdp_md C memory location contents to a file handle fh.
        fh could be sys.stdout, or any other open file handle.
        Note: This just calls the corresponding C library function which
        handles the printing, details, and indentation.
        """

        # need to convert this file handle to a C FILE*
        __fh = PyFile_AsFile(fh)
        __func = gdp.gdp_md_dump
        __func.argtypes = [POINTER(self.gdp_md_t), FILE_P, c_int, c_int]
        # ignore the return value

        __func(self.ptr, __fh, detail, indent)


    def add(self, md_id, data):
        """ Add a new entry to the metadata set """

        __func = gdp.gdp_md_add
        __func.argtypes = [POINTER(self.gdp_md_t), gdp_md_id_t,
                                c_size_t, c_void_p]
        __func.restype = EP_STAT

        size = c_size_t(len(data))
        tmp_buf = create_string_buffer(data, len(data))

        estat = __func(self.ptr, gdp_md_id_t(md_id),
                                size, byref(tmp_buf))
        check_EP_STAT(estat)


    def get(self, index):
        """ Get a new entry from the metadata set by index """

        __func = gdp.gdp_md_get
        __func.argtypes = [POINTER(self.gdp_md_t), c_int,
                                POINTER(gdp_md_id_t),
                                POINTER(c_size_t), POINTER(c_void_p)]
        __func.restype = EP_STAT

        md_id = gdp_md_id_t()
        dlen = c_size_t()
        data_ptr = c_void_p()
        _index = c_int(index)

        estat = __func(self.ptr, _index, byref(md_id),
                                byref(dlen), byref(data_ptr))
        check_EP_STAT(estat)

        data = string_at(data_ptr, dlen.value)

        return (md_id.value, data)


    def find(self, md_id):
        """ Get a new entry from the metadata set by id """

        __func = gdp.gdp_md_find
        __func.argtypes = [POINTER(self.gdp_md_t), gdp_md_id_t,
                                POINTER(c_size_t), POINTER(c_void_p)]
        __func.restype = EP_STAT

        dlen = c_size_t()
        data_ptr = c_void_p()

        estat = __func(self.ptr, gdp_md_id_t(md_id),
                                byref(dlen), byref(data_ptr))
        check_EP_STAT(estat)

        data = string_at(data_ptr, dlen.value)

        return data
