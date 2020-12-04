/* vim: set ai sw=4 sts=4 ts=4 : */

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
#include <ep_crypto.h>
#include <ep_dbg.h>

#include <openssl/conf.h>
#include <openssl/engine.h>
#include <openssl/err.h>


static EP_DBG	Dbg = EP_DBG_INIT("libep.crypto", "crypto support");

/*
**  LIBEP CRYPTOGRAPHIC SUPPORT
*/


/*
**  Initialize crypto support.
**		This basically loads everything.  If running in a small
**		address space you might want to adjust this.
*/

void
ep_crypto_init(uint32_t flags)
{
	static bool initialized = false;

	if (initialized)
		return;
	OPENSSL_load_builtin_modules();
	OpenSSL_add_all_algorithms();

	if (ep_adm_getboolparam("libep.crypto.dev", true))
		ENGINE_load_cryptodev();
	initialized = true;
}


/*
**  Internal helper for printing errors.
*/

EP_STAT
_ep_crypto_error(EP_STAT def_stat, const char *msg, ...)
{
	static bool initialized = false;
	EP_STAT estat = def_stat;
	unsigned long ssl_err;

	// try to decode SSL error codes (too many to count)
	ssl_err = ERR_peek_error();
	switch (ERR_GET_REASON(ssl_err))
	{
	 case UI_R_RESULT_TOO_SMALL:
		estat = EP_STAT_CRYPTO_TOOSMALL;
		break;

	 case UI_R_RESULT_TOO_LARGE:
		estat = EP_STAT_CRYPTO_TOOLARGE;
		break;
	}

	if (ep_dbg_test(Dbg, 17))
	{
		char ebuf[200];

		ep_dbg_printf("_ep_crypto_error: SSL error %ld => %s\n",
				ssl_err, ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	if (!ep_dbg_test(Dbg, 9))
		return estat;

	// load openssl error strings if not already done
	if (!initialized)
	{
		ERR_load_crypto_strings();
		initialized = true;
	}

	FILE *fp = ep_dbg_getfile();
	va_list ap;
	va_start(ap, msg);
	ep_dbg_printf("EP Crypto Error: ");
	vfprintf(fp, msg, ap);
	char ebuf[100];
	ep_dbg_printf("\n    %s\n", ep_stat_tostr(estat, ebuf, sizeof ebuf));
	ERR_print_errors_fp(fp);
	if (ep_dbg_test(Dbg, 31))
		ep_dbg_backtrace(NULL);

	return estat;
}


#if !EP_OSCF_HAS_ARC4RANDOM
# include "ep_assert.h"
# include <fcntl.h>
# include <string.h>
#endif

void
ep_crypto_random_buf(void *buf, size_t n)
{
#if EP_OSCF_HAS_ARC4RANDOM
	arc4random_buf(buf, n);
#else
	const char *randfile = "/dev/urandom";
	int rfd = open(randfile, O_RDONLY);
	if (rfd < 0)
	{
		randfile = "/dev/random";
		rfd = open(randfile, O_RDONLY);
	}
	if (rfd < 0)
	{
		EP_ASSERT_FAILURE("ep_crypto_random_buf: cannot open /dev/urandom or /dev/random: %s",
					strerror(errno));
		abort();
	}
	int i = read(rfd, buf, n);
	if (i < n)
	{
		EP_ASSERT_FAILURE("ep_crypto_random_buf: cannot read %s (%d): %s",
					randfile, i, strerror(errno));
		abort();
	}
	close(rfd);
#endif
}
