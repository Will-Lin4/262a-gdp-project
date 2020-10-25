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
from EP_CRYPTO import *

class GDP_OPEN_INFO:

    """
    Information that is required to open a GCL -- for internal use only.
    The C equivalent exposed to python users is of a dictionary.

    """

    class gdp_open_info_t(Structure):
        pass


    def __init__(self, open_info={}):
        """
        Constructor: calls the C-function to allocate memory.
            Also sets up various things (such as private signature key)
            into this structure

        The (incomplete) list of the keys, their description, and the
            associated functions for the dictionary open_info is:

        * skey: an instance of the class EP_CRYPTO_KEY.
                Calls C library's gdp_open_info_set_signing_key
        """

        # allocate memory for the C structure
        __func1 = gdp.gdp_open_info_new
        __func1.argtypes = []
        __func1.restype = POINTER(self.gdp_open_info_t)

        self.ptr = __func1()

        # read the open_info dictionary and set appropriate fields
        #   based on the key name.

        if 'skey' in open_info.keys():
            skey = open_info['skey']
            __func2 = gdp.gdp_open_info_set_signing_key
            __func2.argtypes = [POINTER(self.gdp_open_info_t),
                                    POINTER(EP_CRYPTO_KEY.EP_CRYPTO_KEY)]

            __func2.restype = EP_STAT

            estat = __func2(self.ptr, skey.key_ptr)
            check_EP_STAT(estat)


    def __del__(self):
        """
        Destructor: Frees the C structure
        """

        __func = gdp.gdp_open_info_free
        __func.argtypes = [POINTER(self.gdp_open_info_t)]

        __func(self.ptr)
