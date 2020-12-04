/* vim: set ai sw=4 sts=4 ts=4 : */

/*
**  ----- BEGIN LICENSE BLOCK -----
**	GDP: Global Data Plane Support Library
**	From the Ubiquitous Swarm Lab, 490 Cory Hall, U.C. Berkeley.
**
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
*/

#include <ep/ep.h>
#include <ep/ep_crypto.h>
#include <ep/ep_dbg.h>
#include <ep/ep_string.h>

#include "gdp.h"

#include <string.h>

static EP_DBG	Dbg = EP_DBG_INIT("gdp.crypto", "cryptographic operations for GDP");


/**********  Support for hash functions  **********/

/*
**  Allocate/Free/Reset hashes
**		Resetting leaves the data structure in an empty state.
*/

gdp_hash_t *
gdp_hash_new(int alg, void *hashbytes, size_t hashlen)
{
	gdp_buf_t *hashbuf = gdp_buf_new();
	if (hashbytes != NULL)
		gdp_buf_write(hashbuf, hashbytes, hashlen);
	return (gdp_hash_t *) hashbuf;
}

void
gdp_hash_free(gdp_hash_t *hash)
{
	gdp_buf_free((gdp_buf_t *) hash);
}

void
gdp_hash_reset(gdp_hash_t *hash)
{
	gdp_buf_reset((gdp_buf_t *) hash);
}

size_t
gdp_hash_getlength(gdp_hash_t *hash)
{
	return gdp_buf_getlength((gdp_buf_t *) hash);
}

void
gdp_hash_set(gdp_hash_t *hash, void *hashbytes, size_t hashlen)
{
	gdp_buf_t *hashbuf = (gdp_buf_t *) hash;
	gdp_buf_reset(hashbuf);
	gdp_buf_write(hashbuf, hashbytes, hashlen);
}

void *
gdp_hash_getptr(gdp_hash_t *hash, size_t *hashlen_ptr)
{
	gdp_buf_t *hashbuf = (gdp_buf_t *) hash;
	size_t l = gdp_buf_getlength(hashbuf);
	if (hashlen_ptr != NULL)
		*hashlen_ptr = l;
	return gdp_buf_getptr(hashbuf, l);
}

gdp_buf_t *
_gdp_hash_getbuf(gdp_hash_t *hash)
{
	return (gdp_buf_t *) hash;
}


bool
gdp_hash_equal(const gdp_hash_t *a1, const gdp_hash_t *b1)
{
	gdp_buf_t *a = (gdp_buf_t *) a1;
	gdp_buf_t *b = (gdp_buf_t *) b1;
	size_t l = gdp_buf_getlength(a);

	return (l == gdp_buf_getlength(b)) &&
		(memcmp(gdp_buf_getptr(a, l), gdp_buf_getptr(b, l), l) == 0);
}


/**********  Support for signatures  **********/

/*
**  Allocate/Free/Reset signatures
*/

gdp_sig_t *
gdp_sig_new(int al, void *sigbytes, size_t siglen)
{
	gdp_buf_t *sigbuf = gdp_buf_new();
	if (sigbytes != NULL)
		gdp_buf_write(sigbuf, sigbytes, siglen);
	return (gdp_sig_t *) sigbuf;
}

void
gdp_sig_free(gdp_sig_t *sig)
{
	gdp_buf_free((gdp_buf_t *) sig);
}

void
gdp_sig_reset(gdp_sig_t *sig)
{
	gdp_buf_reset((gdp_buf_t *) sig);
}

size_t
gdp_sig_getlength(gdp_sig_t *sig)
{
	return gdp_buf_getlength((gdp_buf_t *) sig);
}

void *
gdp_sig_getptr(gdp_sig_t *sig, size_t *siglen)
{
	gdp_buf_t *sigbuf = (gdp_buf_t *) sig;
	size_t l = gdp_buf_getlength(sigbuf);
	if (siglen != NULL)
		*siglen = l;
	return gdp_buf_getptr(sigbuf, l);
}

gdp_buf_t *
_gdp_sig_getbuf(gdp_sig_t *sig)
{
	return (gdp_buf_t *) sig;
}

gdp_sig_t *
gdp_sig_dup(gdp_sig_t *sig)
{
	return (gdp_sig_t *) gdp_buf_dup((gdp_buf_t *) sig);
}

void
gdp_sig_copy(gdp_sig_t *from, gdp_sig_t *to)
{
	gdp_buf_reset((gdp_buf_t *) to);
	gdp_buf_copy((gdp_buf_t *) from, (gdp_buf_t *) to);
}

void
gdp_sig_set(gdp_sig_t *sig, void *sigbytes, size_t siglen)
{
	gdp_buf_reset((gdp_buf_t *) sig);
	gdp_buf_write((gdp_buf_t *) sig, sigbytes, siglen);
}



/**********  Miscellaneous support  **********/

/*
**  Read a secret key
**
**  This should have a better way of handling password-protected keys.
**  At the moment it uses the OpenSSL built-in "read from keyboard"
**  method.
*/

EP_CRYPTO_KEY *
_gdp_crypto_skey_read(const char *searchpath, const char *filename)
{
	char *p = NULL;
	char *dir;
	int keyform = EP_CRYPTO_KEYFORM_PEM;
	int i;

	ep_dbg_cprintf(Dbg, 39, "_gdp_crypto_skey_read(%s)\n", filename);

	if ((i = strlen(filename)) > 4)
	{
		if (strcmp(&filename[i - 4], ".pem") == 0)
			keyform = EP_CRYPTO_KEYFORM_PEM;
		else if (strcmp(&filename[i - 4], ".der") == 0)
			keyform = EP_CRYPTO_KEYFORM_DER;
	}

	if (filename[0] == '/')
	{
		// absolute path name
		searchpath = "";
	}
	else if (searchpath == NULL)
	{
		// get the default search path
		searchpath = ep_adm_getstrparam("swarm.gdp.crypto.key.path",
				"."
				":KEYS"
				":~/.swarm/gdp/keys"
				":/usr/local/etc/gdp/keys"
				":/usr/local/etc/swarm/gdp/keys"
				":/etc/gdp/keys"
				":/etc/swarm/gdp/keys");
	}

	char pbuf[strlen(searchpath) + 1];
	strlcpy(pbuf, searchpath, sizeof pbuf);
	for (dir = pbuf; dir != NULL; dir = p)
	{
		FILE *fp;
		EP_CRYPTO_KEY *key;
		char fnbuf[512];

		p = strchr(dir, ':');
		if (p != NULL)
			*p++ = '\0';
		if (*dir == '\0' && filename[0] != '/')
			continue;

		// create the candidate filename
		if (strncmp(dir, "~/", 2) == 0)
		{
			char *home = getenv("HOME");

			if (home == NULL)
				continue;
			snprintf(fnbuf, sizeof fnbuf, "%s%s/%s", home, dir + 1, filename);
		}
		else
		{
			snprintf(fnbuf, sizeof fnbuf, "%s/%s", dir, filename);
		}

		ep_dbg_cprintf(Dbg, 40,
				"_gdp_crypto_skey_read: trying %s\n",
				fnbuf);
		fp = fopen(fnbuf, "r");
		if (fp == NULL)
			continue;

		key = ep_crypto_key_read_fp(fp, fnbuf, keyform, EP_CRYPTO_F_SECRET);
		ep_dbg_cprintf(Dbg, 40,
				"_gdp_crypto_skey_read: found file, key = %p\n",
				key);
		fclose(fp);
		if (key != NULL)
			return key;
	}
	return NULL;
}
