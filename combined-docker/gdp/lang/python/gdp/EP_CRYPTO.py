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


## EP Crypto stuff, primarily key management

# define EP_CRYPTO_MAX_PUB_KEY  (1024 * 8)
# define EP_CRYPTO_MAX_SEC_KEY  (1024 * 8)
# define EP_CRYPTO_MAX_DIGEST   (512 / 8)
# define EP_CRYPTO_MAX_DER  (1024 * 8)  //XXX should add a slop factor

EP_CRYPTO_MAX_PUB_KEY = (1024 * 8)
EP_CRYPTO_MAX_SEC_KEY = (1024 * 8)
EP_CRYPTO_MAX_DIGEST = (512 / 8)
EP_CRYPTO_MAX_DER = (1024 * 8)  # //XXX should add a slop factor

# define EP_CRYPTO_KEY      EVP_PKEY

# // on-disk key formats
# define EP_CRYPTO_KEYFORM_UNKNOWN  0   // error
# define EP_CRYPTO_KEYFORM_PEM      1   // PEM (text)
# define EP_CRYPTO_KEYFORM_DER      2   // DER (binary ASN.1)
(EP_CRYPTO_KEYFORM_UNKNOWN, EP_CRYPTO_KEYFORM_PEM, EP_CRYPTO_KEYFORM_DER) = \
                                                (0, 1, 2)

# // asymmetric key types
# define EP_CRYPTO_KEYTYPE_UNKNOWN  0   // error
# define EP_CRYPTO_KEYTYPE_RSA      1   // RSA
# define EP_CRYPTO_KEYTYPE_DSA      2   // DSA
# define EP_CRYPTO_KEYTYPE_EC       3   // Elliptic curve
# define EP_CRYPTO_KEYTYPE_DH       4   // Diffie-Hellman
(EP_CRYPTO_KEYTYPE_UNKNOWN, EP_CRYPTO_KEYTYPE_RSA, EP_CRYPTO_KEYTYPE_DSA, \
         EP_CRYPTO_KEYTYPE_EC, EP_CRYPTO_KEYTYPE_DH) = (0, 1, 2, 3, 4)


# // symmetric key type (used to encrypt secret asymmetric keys)
# define EP_CRYPTO_SYMKEY_NONE      0   // error/unencrypted, MBZ
# define EP_CRYPTO_SYMKEY_AES128    1   // Advanced Encr Std 128
# define EP_CRYPTO_SYMKEY_AES192    2   // Advanced Encr Std 192
# define EP_CRYPTO_SYMKEY_AES256    3   // Advanced Encr Std 256
# define EP_CRYPTO_SYMKEY_CAMELLIA128   4   // Camellia 128
# define EP_CRYPTO_SYMKEY_CAMELLIA192   5   // Camellia 192
# define EP_CRYPTO_SYMKEY_CAMELLIA256   6   // Camellia 256
# define EP_CRYPTO_SYMKEY_DES       7   // single Data Encr Std
# define EP_CRYPTO_SYMKEY_3DES      8   // triple Data Encr Std
# define EP_CRYPTO_SYMKEY_IDEA      9   // Int'l Data Encr Alg

(EP_CRYPTO_SYMKEY_NONE, EP_CRYPTO_SYMKEY_AES128, EP_CRYPTO_SYMKEY_AES192,     \
        EP_CRYPTO_SYMKEY_AES256, EP_CRYPTO_SYMKEY_CAMELLIA128,                \
        EP_CRYPTO_SYMKEY_CAMELLIA192, EP_CRYPTO_SYMKEY_CAMELLIA256,           \
        EP_CRYPTO_SYMKEY_DES, EP_CRYPTO_SYMKEY_3DES, EP_CRYPTO_SYMKEY_IDEA) = \
            (0, 1, 2, 3, 4, 5, 6, 7, 8, 9)

# define EP_CRYPTO_SYMKEY_MASK      0xff
EP_CRYPTO_SYMKEY_MASK = 0xff

# // flag bits
# define EP_CRYPTO_F_PUBLIC     0x0000  // public key (no flags set)
# define EP_CRYPTO_F_SECRET     0x0001  // secret key
(EP_CRYPTO_F_PUBLIC, EP_CRYPTO_F_SECRET) = (0x0000, 0x0001)


# // limits
# define EP_CRYPTO_KEY_MINLEN_RSA   1024
EP_CRYPTO_KEY_MINLEN_RSA = 1024


class EP_CRYPTO_KEY:
    """ 
    A generic class to represent a priavte/public encryption/signature key

    Note that either an explicit call to ep_crypto_init or a call to gdp_init
        (which, in turn, calls the ep_crypto_init) needs to be made before 
        using any of this... Especially where we are reading data from file
    """

    class EP_CRYPTO_KEY(Structure):
        """ corresponds to the C type EP_CRYPTO_KEY"""
        pass


    def __init__(self, **kwargs):

        """ 
        Key setup: There are several factors to be considered. As of now, the 
            C library supports multiple ways for key initialization. In python,
            we support the initialization in the following order:
            * Read from a memory buffer.
            * Read from a file-pointer
            * Read from a filename
        We also need some other information about the key (which probably will
            go away in the future). As of now, we need
            * keyform: EP_CRYPTO_KEYFORM_PEM or EP_CRYPTO_KEYFORM_DER, 
            * flags: to specify EP_CRYPTO_F_PUBLIC and/or EP_CRYPTO_F_SECRET

        """
        self.keyform = kwargs.get("keyform", EP_CRYPTO_KEYFORM_UNKNOWN)
        self.flags = c_uint32(kwargs.get("flags", 0x0000))

        if "buf" in kwargs.keys():

            # we have a binary string that represents the key
            # XXX: do we need null terminated strings? Probably not
            #      Passing explicit length takes care of that
            mbuf = create_string_buffer(buf, len(buf))

            __func = gdp.ep_crypto_key_read_mem
            __func.argtypes = [c_void_p, c_size_t, c_int, c_uint32]
            __func.restype = POINTER(self.EP_CRYPTO_KEY)

            self.key_ptr = __func(mbuf, len(mbuf), self.keyform, self.flags)


        elif "fp" in kwargs.keys():

            # we are passed an open file pointer (Python)
            __func = gdp.ep_crypto_key_read_fp
            __func.argtypes = [FILE_P, c_char_p, c_int, c_unit32]
            __func.restype = POINTER(self.EP_CRYPTO_KEY) 

            # "filename" is an optional parameter if we are passed a file
            #   pointer. The C interface uses it only for debugging info
            fbuf = create_string_buffer(kwargs.get('filename', ""))
            fp = PyFile_AsFile(kwargs['fp'])
            self.key_ptr = __func(fp, fbuf, self.keyform, self.flags)


        elif "filename" in kwargs.keys(): 

            # we read the filename to initialize
            __func = gdp.ep_crypto_key_read_file
            __func.argtypes = [c_char_p, c_int, c_uint32]
            __func.restype = POINTER(self.EP_CRYPTO_KEY) 

            fbuf = create_string_buffer(kwargs['filename'])
            self.key_ptr = __func(fbuf, self.keyform, self.flags)

        else: 
            # we don't know what to do. We don't have any keydata
            raise Exception


    def __del__(self):
        """ Destructor. Free up the corresponding C structure"""

        __func = gdp.ep_crypto_key_free
        __func.argtypes = [POINTER(self.EP_CRYPTO_KEY)]

        __func(self.key_ptr)
