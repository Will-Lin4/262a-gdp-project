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

/*
**  Do cryptographic signatures
**
**	Note that the internal structure while computing a signature
**	is an EP_CRYPTO_MD; thus you may see a mix of calls to (for
**	example) ep_crypto_md_clone and ep_crypto_sign_final.
*/

#include <ep/ep.h>
#include <ep/ep_crypto.h>
#include <ep/ep_dbg.h>

static EP_DBG	Dbg = EP_DBG_INIT("libep.crypto.sign", "cryptographic signature generation");


/*
**  Create a new signing context.  Note that this returns a message
**  digest type; there isn't a separate type for signing.
*/

EP_STAT
ep_crypto_sign_new(EP_CRYPTO_KEY *skey, int md_alg_id, EP_CRYPTO_MD **pmd)
{
	EP_STAT estat = EP_STAT_OK;
	EP_CRYPTO_MD *md = NULL;
	const EVP_MD *md_alg;
	int istat;

	md_alg = _ep_crypto_md_getalg_byid(md_alg_id);
	if (md_alg == NULL)
	{
		estat = _ep_crypto_error(EP_STAT_CRYPTO_HASHALG,
				"ep_crypto_sign_new: "
				"unknown digest algorithm %d",
				md_alg_id);
		goto fail0;
	}
	md = EVP_MD_CTX_create();
	if (md == NULL)
	{
		estat = _ep_crypto_error(EP_STAT_CRYPTO_DIGEST,
				"ep_crypto_sign_new: "
				"cannot create message digest for signing");
		goto fail0;
	}
	istat = EVP_DigestSignInit(md, NULL, md_alg, NULL, skey);
	if (istat != 1)
	{
		ep_crypto_md_free(md);
		estat = _ep_crypto_error(EP_STAT_CRYPTO_SIGN,
				"ep_crypto_sign_new: "
				"cannot initialize digest for signing");
		goto fail0;
	}
fail0:
	if (ep_dbg_test(Dbg, 64))
	{
		char ebuf[100];
		ep_dbg_printf("ep_crypto_sign_new(%d) => %p (%s)\n",	//DEBUG
			md_alg_id, md,					//DEBUG
			ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	*pmd = md;
	return estat;
}


/*
**  Update a signing context.  This adds data to be signed.
*/

EP_STAT
ep_crypto_sign_update(EP_CRYPTO_MD *md, void *dbuf, size_t dbufsize)
{
	int istat;

	ep_dbg_cprintf(Dbg, 64, "ep_crypto_sign_update(%p, %zd)\n",	//DEBUG
			md, dbufsize);					//DEBUG
	istat = EVP_DigestSignUpdate(md, dbuf, dbufsize);
	if (istat != 1)
	{
		(void) _ep_crypto_error(EP_STAT_CRYPTO_SIGN,
				"ep_crypto_sign_update: "
				"cannot update signing digest");
	}
	return EP_STAT_OK;
}


/*
**  Finalize a signing context.
**
**	This returns the signature in sbuf and sbufsize, with
**	*sbufsize set to the size of sbuf on call, and modified
**	to be the number of bytes actually used.
**
**	The context cannot be used again.
*/

EP_STAT
ep_crypto_sign_final(EP_CRYPTO_MD *md, void *_sbuf, size_t *sbufsize)
{
	int istat;
	uint8_t *sbuf = (uint8_t *) _sbuf;

	ep_dbg_cprintf(Dbg, 64, "ep_crypto_sign_final(%p, %zd)\n",	//DEBUG
			md, *sbufsize);					//DEBUG
	istat = EVP_DigestSignFinal(md, sbuf, sbufsize);
	if (istat != 1)
	{
		return _ep_crypto_error(EP_STAT_CRYPTO_SIGN,
				"ep_crypto_sign_final: "
				"cannot finalize signing digest");
	}
	return EP_STAT_OK;
}


/*
**  Free the signing context.
*/

void
ep_crypto_sign_free(EP_CRYPTO_MD *md)
{
	EVP_MD_CTX_destroy(md);
}
