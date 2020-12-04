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


class GDP_NAME:

    """
    Represents a GDP name. Each GDP name has potentially up to three
    different representations:
     - a 256 bit binary version
     - a Base-64(ish) representation
     - (optional) a more memorable version; for the moment, this is
       hashed to get the binary version
    """

    # name_t, pname_t are ctypes types that correspond to
    # gdp_name_t and gdp_pname_t, respectively.

    # Types that will be used for some type-checking #
    name_t = c_uint8 * 32     # internal name of a GCL
    PNAME_LEN = 43            # this is following the convention in gdp.h
    pname_t = c_uint8 * (PNAME_LEN + 1)     # printable name of a GCL

    def __init__(self, name, force_internal=False):
        """
        takes either an internal name, or a printable name, or a human friendly
            name, and creates a python object that can be passed around for the
            various GDP calls
        """

        def __get_printable_name(internal_name):

            # ctypes magic to create an array representing gdp_name_t
            buf1 = create_string_buffer(internal_name, 32)
            name_t_ptr = cast(byref(buf1), POINTER(self.name_t))

            # ctypes magic to create an array representing gdp_pname_t
            buf2 = create_string_buffer(self.PNAME_LEN + 1)
            pname_t_ptr = cast(byref(buf2), POINTER(self.pname_t))

            # for strict type checking
            __func = gdp.gdp_printable_name
            __func.argtypes = [self.name_t, self.pname_t]
            __func.restype = c_char_p

            res = __func(name_t_ptr.contents, pname_t_ptr.contents)
            # the return of __func is just a pointer to the printable_name
            return string_at(pname_t_ptr.contents, self.PNAME_LEN + 1)

        def __parse_name(name):

            buf1 = create_string_buffer(name, len(name)+1)
            buf2 = create_string_buffer(32)
            name_t_ptr = cast(byref(buf2), POINTER(self.name_t))

            __func = gdp.gdp_parse_name
            __func.argtypes = [c_char_p, self.name_t]
            __func.restype = EP_STAT

            estat = __func(buf1, name_t_ptr.contents)
            check_EP_STAT(estat)
            return string_at(name_t_ptr.contents, 32)

        def __is_binary_string(s):
            textchars = bytearray({7,8,9,10,12,13,27} | \
                            set(range(0x20, 0x100)) - {0x7f})
            return bool(str(s).translate(None, textchars))


        if len(name) == 32 and (__is_binary_string(name) or force_internal):
            # If length of name is exactly 32, treat it as a gdp_name_t
            # This is bit guesswork

            self.name = name                                # Python string
            self.pname = __get_printable_name(self.name)    # Python string

        else:
            # we invoke parse_name, which gives us the internal name.
            self.name = __parse_name(name)
            self.pname = __get_printable_name(self.name)

    def is_valid(self):
        "Borrowed from the GDP C api, tells if the name is valid or not"

        # We could potentially just return it by reimplementing the logic in
        #   gdp_name_is_valid, but then we could theoretically reimplement
        #   everything in python too.
        # ctypes magic to create an array representing gdp_name_t
        buf1 = create_string_buffer(self.name, 32)
        name_t_ptr = cast(byref(buf1), POINTER(self.name_t))

        __func = gdp.gdp_name_is_valid
        __func.argtypes = [self.name_t]
        __func.restype = c_bool

        return __func(name_t_ptr.contents)

    def printable_name(self):
        "Returns a python string that represents the printable name"
        return self.pname                                   # Python string

    def internal_name(self):
        "Returns a python string that represents the internal binray name"
        return self.name                                    # Python string
