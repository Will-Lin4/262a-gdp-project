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


"""
A Python API for libgdp. Uses ctypes and shared library versions of
libgdp and libep.

This package exports a number of classes, and a few utility functions.
Here is a very brief summary, but please see the detailed C-api docs.

- GDP_NAME:      a name in GDP
- GDP_GIN :      a GDP instance
- GDP_EVENT:     an asynchronous event
- GDP_DATUM:     a GDP datum: (kind of) a record in memory
- GDP_HASH:      a cryptographic hash
- GDP_SIG:       a signature
- GDP_BUF:       a generic buffer
- GDP_MD:	     a metadata object
- GDP_OPEN_INFO: information regarding metadata

Note that a number of methods take a NULL/None value, which implies
that the underlying C-library makes a decision on what value to
use (based on configuration parameters or defaults). Providing a
non-null argument is simply a way to override these defaults.

"""

from MISC import \
        GDP_MODE_ANY, GDP_MODE_RO, GDP_MODE_AO, GDP_MODE_RA, \
        _GDP_EVENT_FREE, GDP_EVENT_DATA, \
        GDP_EVENT_DONE, GDP_EVENT_SHUTDOWN, \
        GDP_EVENT_CREATED, GDP_EVENT_SUCCESS, \
        GDP_EVENT_FAILURE, GDP_EVENT_MISSING, \
        GDP_MD_XID, GDP_MD_PUBKEY, GDP_MD_CTIME, \
        GDP_MD_EXPIRE, GDP_MD_CID, GDP_MD_SYNTAX, \
        GDP_MD_LOCATION, GDP_MD_UUID, \
        gdp_init, gdp_run_accept_event_loop, \
        dbg_set, check_EP_STAT, ep_stat_tostr, get_current_ts
from GDP_DATUM import GDP_DATUM_PRTEXT, GDP_DATUM_PRDEBUG, \
        GDP_DATUM_PRSIG, GDP_DATUM_PRQUIET, GDP_DATUM_PRMETAONLY, \
        GDP_DATUM
from GDP_NAME import GDP_NAME
from GDP_GIN import GDP_GIN
from GDP_EVENT import GDP_EVENT
from GDP_BUF import GDP_BUF
from GDP_HASH import GDP_HASH
from GDP_SIG import GDP_SIG
from GDP_MD import GDP_MD
from EP_CRYPTO import EP_CRYPTO_KEY, \
    EP_CRYPTO_KEYFORM_UNKNOWN, EP_CRYPTO_KEYFORM_PEM, EP_CRYPTO_KEYFORM_DER, \
    EP_CRYPTO_F_PUBLIC, EP_CRYPTO_F_SECRET
from GDPcache import GDPcache

__all__ = [GDP_MODE_ANY, GDP_MODE_RO, GDP_MODE_AO, GDP_MODE_RA,
           _GDP_EVENT_FREE, GDP_EVENT_DATA, GDP_EVENT_DONE,
           GDP_EVENT_SHUTDOWN, GDP_EVENT_CREATED,
           GDP_EVENT_SUCCESS, GDP_EVENT_FAILURE, GDP_EVENT_MISSING,
           GDP_MD_XID, GDP_MD_PUBKEY, GDP_MD_CTIME, GDP_MD_EXPIRE,
           GDP_MD_CID, GDP_MD_SYNTAX, GDP_MD_LOCATION, GDP_MD_UUID,
           gdp_init, gdp_run_accept_event_loop, dbg_set,
           check_EP_STAT, ep_stat_tostr, get_current_ts,
           GDP_DATUM_PRTEXT, GDP_DATUM_PRDEBUG,
           GDP_DATUM_PRSIG, GDP_DATUM_PRQUIET, GDP_DATUM_PRMETAONLY,
           GDP_DATUM,
           GDP_NAME, GDP_GIN,
           GDP_EVENT, GDP_BUF, GDP_HASH, GDP_SIG, GDP_MD,
           EP_CRYPTO_KEY,
           EP_CRYPTO_KEYFORM_UNKNOWN, EP_CRYPTO_KEYFORM_PEM,
           EP_CRYPTO_KEYFORM_DER,
           EP_CRYPTO_F_PUBLIC, EP_CRYPTO_F_SECRET,
           GDPcache ]
