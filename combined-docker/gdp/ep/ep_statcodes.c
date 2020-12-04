/* vim: set ai sw=8 sts=8 ts=8 : */

/***********************************************************************
**  ----- BEGIN LICENSE BLOCK -----
**	LIBEP: Enhanced Portability Library (Reduced Edition)
**
**	Copyright (c) 2008-2019, Eric P. Allman.  All rights reserved.
**	Copyright (c) 2015-2019, Regents of the University of California.
**	All rights reserved.
**
**	Permission is hereby granted, without written agreement and without
**	license or royalty fees, to use, copy, modify, and distribute this
**	software and its documentation for any purpose, provided that the above
**	copyright notice and the following two paragraphs appear in all copies
**	of this software.
**
**	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
**	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
**	PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
**	EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
**	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
**	FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION,
**	IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO
**	OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS,
**	OR MODIFICATIONS.
**  ----- END LICENSE BLOCK -----
***********************************************************************/

#include <ep/ep.h>

static struct ep_stat_to_string	Stats[] =
{
    // generic status codes
    { _EP_STAT_INTERNAL(OK, EP_STAT_MOD_GENERIC, 0), "generic"		},

    // individual codes
    { EP_STAT_WARN,		"generic warning",			},
    { EP_STAT_ERROR,		"generic error",			},
    { EP_STAT_SEVERE,		"generic severe error",			},
    { EP_STAT_ABORT,		"generic abort",			},

    { EP_STAT_OUT_OF_MEMORY,	"out of memory",			},
    { EP_STAT_INVALID_ARG,	"invalid argument",			},
    { EP_STAT_END_OF_FILE,	"end of file",				},
    { EP_STAT_TIME_BADFORMAT,	"bad time format",			},
    { EP_STAT_BUF_OVERFLOW,	"potential buffer overflow",		},
    { EP_STAT_TIME_PARSE,	"cannot parse date/time string",	},
    { EP_STAT_DNS_NOTFOUND,	"DNS error: name not found",		},
    { EP_STAT_DNS_FAILURE,	"DNS error: lookup failed",		},
    { EP_STAT_ASSERT_ABORT,	"assertion failure",			},
    { EP_STAT_NULL_POINTER,	"invalid null pointer",			},
    { EP_STAT_NOT_INITIALIZED,	"initialization required",		},
    { EP_STAT_UUID_PARSE_ERROR,	"cannot parse UUID",			},
    { EP_STAT_UUID_VERSION,	"illegal UUID version",			},
    { EP_STAT_SOFTWARE_ERROR,	"internal software error",		},
    { EP_STAT_NOT_IMPLEMENTED,	"not implemented",			},
    { EP_STAT_NOT_FOUND,	"item not found",			},

    // cryptographic status codes
    { _EP_STAT_INTERNAL(OK, EP_STAT_MOD_CRYPTO, 0), "crypto"		},

    { EP_STAT_CRYPTO_DIGEST,	"cryptographic digest failure",		},
    { EP_STAT_CRYPTO_SIGN,	"cryptographic signing failure",	},
    { EP_STAT_CRYPTO_VRFY,	"cryptographic verification failure",	},
    { EP_STAT_CRYPTO_BADSIG,	"cryptographic signature doesn't match",},
    { EP_STAT_CRYPTO_KEYTYPE,	"unknown cryptographic key type",	},
    { EP_STAT_CRYPTO_KEYFORM,	"unknown cryptographic key format",	},
    { EP_STAT_CRYPTO_CONVERT,	"cannot encode/decode cryptographic key", },
    { EP_STAT_CRYPTO_KEYCREATE,	"cannot create new cryptographic key",	},
    { EP_STAT_CRYPTO_KEYCOMPAT,	"incompatible cryptographic keys",	},
    { EP_STAT_CRYPTO_CIPHER,	"symmetric cipher failure",		},
    { EP_STAT_CRYPTO_TOOSMALL,	"key or password too short",		},
    { EP_STAT_CRYPTO_TOOLARGE,	"key or password too long",		},
    { EP_STAT_CRYPTO_FAIL,	"unknown cryptographic failure",	},
    { EP_STAT_CRYPTO_HASHALG,	"unknown message digest algorithm",	},
    { EP_STAT_CRYPTO_KEYFAIL,	"corrupt key",				},

    { EP_STAT_OK,		NULL,					}
};


void
_ep_stat_init(void)
{
    ep_stat_reg_strings(Stats);
}
