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
from GDP_BUF import GDP_BUF

# Formatting options
# GDP_DATUM_PRTEXT        0x00000001      // print data as text
# GDP_DATUM_PRDEBUG       0x00000002      // print debugging info
# GDP_DATUM_PRSIG         0x00000004      // print the signature
# GDP_DATUM_PRQUIET       0x00000008      // don't print any metadata
# GDP_DATUM_PRMETAONLY    0x00000010      // only print metadata
(GDP_DATUM_PRTEXT, GDP_DATUM_PRDEBUG, GDP_DATUM_PRSIG,
        GDP_DATUM_PRQUIET, GDP_DATUM_PRMETAONLY) = (1, 2, 3, 8, 16)


class GDP_DATUM:

    """
    A class only for internal use. The C datum equivalent exposed to
    python users is of a dictionary.

    This is to avoid any side effects that the underlying buffer
    implementation may cause.
    """

    class gdp_datum_t(Structure):
        pass

    def __init__(self, **kwargs):
        """
        Constructor: Two ways of creating a new pythong GDP datum object:
        - Create a new object and allocate new memeory for it by calling
          the C library function
        - Create a new object, but associate it with an existing memory
          location.
        In the first case: we need to free the memory when it is no longer
        needed. In the second case: we shouldn't free memory allocated by
        someone else.
        """
        if len(kwargs) == 0:

            # We do need to create a corresponding C data structure.
            __func = gdp.gdp_datum_new
            __func.argtypes = []
            __func.restype = POINTER(self.gdp_datum_t)

            self.ptr = __func()
            self.did_i_create_it = True

        else:
            # we probably got passed a C pointer to an existing datum.
            if "ptr" in kwargs:
                self.ptr = kwargs["ptr"]
                self.did_i_create_it = False
            else:
                raise Exception     # FIXME

    def __del__(self):
        """
        Destructor: Does nothing if this GDP datum was created by
        passing an exisiting datum pointer
        """

        if self.did_i_create_it:
            __func = gdp.gdp_datum_free
            __func.argtypes = [POINTER(self.gdp_datum_t)]
            __func(self.ptr)


    def print_(self, fh, flags):
        """
        Print the GDP datum C memory location contents to a file handle fh.
        fh could be sys.stdout, or any other open file handle. Flags are
        predefined constants (see GDP_DATUM_PR*)

        Note: This just calls the corresponding C library function which
        handles the printing
        """

        # need to convert this file handle to a C FILE*
        __fh = PyFile_AsFile(fh)
        __func = gdp.gdp_datum_print
        __func.argtypes = [POINTER(self.gdp_datum_t), FILE_P, c_uint32]
        # ignore the return value
        __func(self.ptr, __fh, c_uint32(flags))

    def __getitem__(self, key):

        if key == "recno":
            return self.getrecno()
        if key == "ts":
            return self.getts()
        if key == "dlen":
            return self.getdlen()
        if key == "buf":
            return self.getbuf()
        if key == "sig":
            return self.getsig()
        raise NotImplementedError

    def reset(self):
        """ Reset the datum """
        __func = gdp.gdp_datum_reset
        __func.argtypes = [POINTER(self.gdp_datum_t)]
        __func(self.ptr)


    def gethash(self, gin):
        """ Compute the hash of the datum for a given log? """

        assert isinstance(gin, GDP_GIN)
        __func = gdp.gdp_datum_hash
        __func.argtypes = [POINTER(self.gdp_datum_t),
                                    POINTER(GDP_GIN.gdp_gin_t)]
        __func.restype = POINTER(gdp_hash_t)

        gdp_hash_ptr = __func(self.ptr, gin.ptr)
        return GDP_HASH(ptr=gdp_hash_ptr)


    def getrecno(self):
        """
        Get the corresponding record number associated with this datum
        """

        __func = gdp.gdp_datum_getrecno
        __func.argtypes = [POINTER(self.gdp_datum_t)]
        __func.restype = gdp_recno_t

        ret = __func(self.ptr)
        return int(ret)

    def getts(self):
        """
        Return the timestamp associated with this GDP_DATUM in the form of a
        dictionary. The keys are: tv_sec, tv_nsec and tv_accuracy
        """

        ts = EP_TIME_SPEC()
        __func = gdp.gdp_datum_getts
        __func.argtypes = [
            POINTER(self.gdp_datum_t), POINTER(EP_TIME_SPEC)]
        # ignore the return value

        __func(self.ptr, byref(ts))
        # represent the time spec as a dictionary

        ret = {}
        ret['tv_sec'] = ts.tv_sec
        ret['tv_nsec'] = ts.tv_nsec
        ret['tv_accuracy'] = ts.tv_accuracy

        return ret

    def getdlen(self):
        """
        Returns the length of the data in GDP_BUF associated with
        this GDP_DATUM.
        """

        __func = gdp.gdp_datum_getdlen
        __func.argtypes = [POINTER(self.gdp_datum_t)]
        __func.restype = c_size_t

        ret = __func(self.ptr)
        return ret

    def getbuf(self):
        """ Return the GDP_BUF associated with this GDP_DATUM. """

        __func = gdp.gdp_datum_getbuf
        __func.argtypes = [POINTER(self.gdp_datum_t)]
        __func.restype = POINTER(GDP_BUF.gdp_buf_t)

        gdp_buf_ptr = __func(self.ptr)
        gdp_buf = GDP_BUF(ptr=gdp_buf_ptr)
        gdp_buf.datum = self    # garbage collection workaround
        return gdp_buf


    # def setbuf(self, data):
    #     "Set the buffer to the given data. data is a python string"

    #     __func = gdp.gdp_datum_getbuf
    #     __func.argtypes = [POINTER(self.gdp_datum_t)]
    #     __func.restype = POINTER(GDP_BUF.gdp_buf_t)

    #     gdp_buf_ptr = __func(self.ptr)

    #     __func_write = gdp.gdp_buf_write
    #     __func_write.argtypes = [POINTER(GDP_BUF.gdp_buf_t), c_void_p, c_size_t]
    #     __func_write.restype = c_int

    #     size = c_size_t(len(data))
    #     tmp_buf = create_string_buffer(data, len(data))
    #     written_bytes = __func_write(gdp_buf_ptr, byref(tmp_buf), size)
    #     return


    def getsig(self):
        """ Return the signature as a binary string.
        As of current, the signature is over (recno|data)"""

        # get a pointer to signature buffer
        __func = gdp.gdp_datum_getsig
        __func.argtypes = [POINTER(self.gdp_datum_t)]
        __func.restype = POINTER(GDP_BUF.gdp_buf_t)

        sig_buf_ptr = __func(self.ptr)

        return GDP_SIG(ptr=sig_buf_ptr)
