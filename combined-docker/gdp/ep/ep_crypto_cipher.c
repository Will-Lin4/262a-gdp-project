/* vim: set ai sw=8 sts=8 ts=8 :*/

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
**  Symmetric cipher support.
*/

#include <ep/ep.h>
#include <ep/ep_crypto.h>

struct ep_crypto_cipher_ctx
{
	EP_CRYPTO_KEY	*key;		// symmetric encryption/decryption key
	EVP_CIPHER_CTX	*ctx;		// OpenSSL cipher context
//	uint8_t		iv[EVP_MAX_IV_LENGTH];
};


/*
**  Convert EP cipher code to OpenSSL cipher (helper routine)
*/

static const EVP_CIPHER *
evp_cipher_type(uint32_t cipher)
{
	switch (cipher)
	{
	  case EP_CRYPTO_SYMKEY_AES128 | EP_CRYPTO_MODE_CBC:
		return EVP_aes_128_cbc();
	  case EP_CRYPTO_SYMKEY_AES128 | EP_CRYPTO_MODE_CFB:
		return EVP_aes_128_cfb();
	  case EP_CRYPTO_SYMKEY_AES128 | EP_CRYPTO_MODE_OFB:
		return EVP_aes_128_ofb();

	  case EP_CRYPTO_SYMKEY_AES192 | EP_CRYPTO_MODE_CBC:
		return EVP_aes_192_cbc();
	  case EP_CRYPTO_SYMKEY_AES192 | EP_CRYPTO_MODE_CFB:
		return EVP_aes_192_cfb();
	  case EP_CRYPTO_SYMKEY_AES192 | EP_CRYPTO_MODE_OFB:
		return EVP_aes_192_ofb();

	  case EP_CRYPTO_SYMKEY_AES256 | EP_CRYPTO_MODE_CBC:
		return EVP_aes_256_cbc();
	  case EP_CRYPTO_SYMKEY_AES256 | EP_CRYPTO_MODE_CFB:
		return EVP_aes_256_cfb();
	  case EP_CRYPTO_SYMKEY_AES256 | EP_CRYPTO_MODE_OFB:
		return EVP_aes_256_ofb();
	}

	(void) _ep_crypto_error(EP_STAT_CRYPTO_KEYTYPE,
			"ep_crypto_cipher_new: unknown cipher type 0x%x",
			cipher);
	return NULL;
}


/*
**  Create a new cipher context.  This is used by most of the
**  rest of the cipher routines.
**
**	The 'enc' parameter tells whether this is an encryption or a
**	description operation (true == encrypt).
**
**	XXX need to explain iv.
*/

EP_CRYPTO_CIPHER_CTX *
ep_crypto_cipher_new(
		uint32_t cipher,
		uint8_t *key,
		uint8_t *iv,
		bool enc)
{
	EP_CRYPTO_CIPHER_CTX *ctx;
	const EVP_CIPHER *ciphertype;
	ENGINE *engine = NULL;

	ciphertype = evp_cipher_type(cipher);
	if (ciphertype == NULL)
		return NULL;

	ctx = (EP_CRYPTO_CIPHER_CTX *) ep_mem_zalloc(sizeof *ctx);
#if OPENSSL_VERSION_NUMBER >= 0x10100000	// version 1.1.0
	ctx->ctx = EVP_CIPHER_CTX_new();
#else
	ctx->ctx = ep_mem_malloc(sizeof *ctx->ctx);
	EVP_CIPHER_CTX_init(ctx->ctx);
#endif
	if (EVP_CipherInit_ex(ctx->ctx, ciphertype, engine, key, iv, enc) <= 0)
	{
		ep_crypto_cipher_free(ctx);
		(void) _ep_crypto_error(EP_STAT_CRYPTO_CIPHER,
				"ep_crypto_cipher_new: "
				"cannot initialize for encryption");
		return NULL;
	}

	return ctx;
}


/*
**  Free a cipher context
**
**	This makes sure that sensitive information is cleared from
**	memory before it is released.
*/

void
ep_crypto_cipher_free(EP_CRYPTO_CIPHER_CTX *ctx)
{
#if OPENSSL_VERSION_NUMBER >= 0x10100000	// version 1.1.0
	EVP_CIPHER_CTX_free(ctx->ctx);
#else
	if (EVP_CIPHER_CTX_cleanup(ctx->ctx) <= 0)
		(void) _ep_crypto_error(EP_STAT_CRYPTO_FAIL,
				"cannot cleanup cipher context");
	ep_mem_free(ctx->ctx);
#endif
	ep_mem_free(ctx);
}


/*
**  Do symmetric encryption
**
**	This can be done using ep_crypto_cipher_crypt, which
**	encrypts or decrypts a block of memory and does the final
**	padding in one operation, or by calling ep_crypto_cipher_update
**	on multiple blocks followed by ep_crypto_cipher_final to
**	do the padding.
**
**	In all cases, the value of a successful status code contains
**	the number of bytes actually written to the output.
*/

EP_STAT
ep_crypto_cipher_crypt(
		EP_CRYPTO_CIPHER_CTX *ctx,
		void *_in,
		size_t inlen,
		void *_out,
		size_t outlen)
{
	uint8_t *in = (uint8_t *) _in;
	uint8_t *out = (uint8_t *) _out;
	int olen;
	int rval;

	// must allow room for final padding in output buffer
	if (outlen < inlen + EVP_CIPHER_CTX_key_length(ctx->ctx))
	{
		// potential buffer overflow
		return _ep_crypto_error(EP_STAT_BUF_OVERFLOW,
				"cp_crypto_cipher_crypt: "
				"short output buffer (%d < %d + %d)",
			outlen, inlen, EVP_CIPHER_CTX_key_length(ctx->ctx));
	}

	if (EVP_CipherUpdate(ctx->ctx, (uint8_t *) out, &olen, (uint8_t *) in, inlen) <= 0)
	{
		return _ep_crypto_error(EP_STAT_CRYPTO_CIPHER,
				"ep_crypto_cipher_crypt: "
				"cannot encrypt/decrypt");
	}
	rval = olen;
	out += olen;
	if (EVP_CipherFinal_ex(ctx->ctx, out, &olen) <= 0)
	{
		return _ep_crypto_error(EP_STAT_CRYPTO_CIPHER,
				"ep_crypto_cipher_crypt: "
				"cannot finalize encrypt/decrypt");
	}

	rval += olen;
	EP_ASSERT(rval >= 0);
	return EP_STAT_FROM_INT(rval);
}


EP_STAT
ep_crypto_cipher_update(
		EP_CRYPTO_CIPHER_CTX *ctx,
		void *_in,
		size_t inlen,
		void *_out,
		size_t outlen)
{
	uint8_t *in = (uint8_t *) _in;
	uint8_t *out = (uint8_t *) _out;
	int olen;

	if (EVP_CipherUpdate(ctx->ctx, out, &olen, in, inlen) <= 0)
	{
		return _ep_crypto_error(EP_STAT_CRYPTO_CIPHER,
				"ep_crypto_cipher_update: "
				"cannot encrypt/decrypt");
	}

	EP_ASSERT(olen >= 0 && (unsigned) olen <= outlen);
	return EP_STAT_FROM_INT(olen);
}


EP_STAT
ep_crypto_cipher_final(
		EP_CRYPTO_CIPHER_CTX *ctx,
		void *_out,
		size_t outlen)
{
	uint8_t *out = (uint8_t *) _out;
	int olen;

	// allow room for possible final padding
	if ((ssize_t) outlen < EVP_CIPHER_CTX_key_length(ctx->ctx))
	{
		// potential buffer overflow
		return _ep_crypto_error(EP_STAT_BUF_OVERFLOW,
				"ep_crypto_cipher_final: "
				"short output buffer (%d < %d)",
				outlen, EVP_CIPHER_CTX_key_length(ctx->ctx));
	}

	if (EVP_CipherFinal_ex(ctx->ctx, out, &olen) <= 0)
	{
		return _ep_crypto_error(EP_STAT_CRYPTO_CIPHER,
				"ep_crypto_cipher_final: "
				"cannot finalize encrypt/decrypt");
	}

	EP_ASSERT(olen >= 0);
	return EP_STAT_FROM_INT(olen);
}
