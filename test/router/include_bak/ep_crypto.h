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

#ifndef _EP_CRYPTO_H_
#define _EP_CRYPTO_H_

# include <ep/ep.h>
# include <ep/ep_statcodes.h>


/*
**  At the moment this wraps openssl, but we could conceivably
**  in the future switch to another package, e.g., NaCl.
*/
//------------------------------------------------------------------------------
//	Added for C++ compability
extern "C"{
//------------------------------------------------------------------------------

__BEGIN_DECLS

# include <openssl/evp.h>
# include <openssl/sha.h>
# include <openssl/opensslv.h>


/*
**  Configuration
*/

# ifndef _EP_CRYPTO_INCLUDE_RSA
#  define _EP_CRYPTO_INCLUDE_RSA	1
# endif
# ifndef _EP_CRYPTO_INCLUDE_DSA
#  define _EP_CRYPTO_INCLUDE_DSA	1
# endif
# ifndef _EP_CRYPTO_INCLUDE_EC
#  define _EP_CRYPTO_INCLUDE_EC		1
# endif
# ifndef _EP_CRYPTO_INCLUDE_DH
#  define _EP_CRYPTO_INCLUDE_DH		0
# endif
# ifndef _EP_CRYPTO_INCLUDE_DER
#  define _EP_CRYPTO_INCLUDE_DER	1	// ASN.1
# endif

/*
**  General defines
*/

# define EP_CRYPTO_MAX_PUB_KEY	(1024 * 8)
# define EP_CRYPTO_MAX_SEC_KEY	(1024 * 8)
# define EP_CRYPTO_MAX_DIGEST	(512 / 8)
# define EP_CRYPTO_MAX_DER	(1024 * 8)	//XXX should add a slop factor


void			ep_crypto_init(uint32_t flags);

void			ep_crypto_random_buf(	// return random data
				void *buf,		// into buf
				size_t n);		// this many bytes


/*
**  Key Management
*/

# define EP_CRYPTO_KEY		EVP_PKEY

// on-disk key formats
# define EP_CRYPTO_KEYFORM_UNKNOWN	0	// error
# define EP_CRYPTO_KEYFORM_PEM		1	// PEM (text)
# define EP_CRYPTO_KEYFORM_DER		2	// DER (binary ASN.1)

// asymmetric key types
# define EP_CRYPTO_KEYTYPE_UNKNOWN	-1	// error
# define EP_CRYPTO_KEYTYPE_NULL		0	// NULL (no encryption)
# define EP_CRYPTO_KEYTYPE_RSA		1	// RSA
# define EP_CRYPTO_KEYTYPE_DSA		2	// DSA
# define EP_CRYPTO_KEYTYPE_EC		3	// Elliptic curve
# define EP_CRYPTO_KEYTYPE_DH		4	// Diffie-Hellman

// symmetric key type (used to encrypt secret asymmetric keys)
# define EP_CRYPTO_SYMKEY_NONE		0	// error/unencrypted, MBZ
# define EP_CRYPTO_SYMKEY_AES128	1	// Advanced Encr Std 128
# define EP_CRYPTO_SYMKEY_AES192	2	// Advanced Encr Std 192
# define EP_CRYPTO_SYMKEY_AES256	3	// Advanced Encr Std 256
# define EP_CRYPTO_SYMKEY_CAMELLIA128	4	// Camellia 128
# define EP_CRYPTO_SYMKEY_CAMELLIA192	5	// Camellia 192
# define EP_CRYPTO_SYMKEY_CAMELLIA256	6	// Camellia 256
# define EP_CRYPTO_SYMKEY_DES		7	// single Data Encr Std
# define EP_CRYPTO_SYMKEY_3DES		8	// triple Data Encr Std
# define EP_CRYPTO_SYMKEY_IDEA		9	// Int'l Data Encr Alg

# define EP_CRYPTO_SYMKEY_MASK		0xff

// flag bits
# define EP_CRYPTO_F_PUBLIC		0x0000	// public key (no flags set)
# define EP_CRYPTO_F_SECRET		0x0001	// secret key

// limits
# define EP_CRYPTO_KEY_MINLEN_RSA	1024

void			ep_crypto_key_print(
				EP_CRYPTO_KEY *key,
				FILE *fp,
				uint32_t flags);

EP_CRYPTO_KEY		*ep_crypto_key_create(
				int keytype,
				int keylen,
				int keyexp,
				const char *curve);
void			ep_crypto_key_free(
				EP_CRYPTO_KEY *key);

EP_CRYPTO_KEY		*ep_crypto_key_read_file(
				const char *filename,
				int keyform,
				uint32_t flags);
EP_CRYPTO_KEY		*ep_crypto_key_read_fp(
				FILE *fp,
				const char *filename,
				int keyform,
				uint32_t flags);
EP_CRYPTO_KEY		*ep_crypto_key_read_mem(
				const void *buf,
				size_t buflen,
				int keyform,
				uint32_t flags);

EP_STAT			ep_crypto_key_write_file(
				EP_CRYPTO_KEY *key,
				const char *filename,
				int keyform,
				int keyenc,
				const char *passwd,
				uint32_t flags);
EP_STAT			ep_crypto_key_write_fp(
				EP_CRYPTO_KEY *key,
				FILE *fp,
				int keyform,
				int keyenc,
				const char *passwd,
				uint32_t flags);
EP_STAT			ep_crypto_key_write_mem(
				EP_CRYPTO_KEY *key,
				void *buf,
				size_t bufsize,
				int keyform,
				int keyenc,
				const char *passwd,
				uint32_t flags);

int			ep_crypto_keyenc_byname(
				const char *enc_alg_str);
const char		*ep_crypto_keyenc_name(
				int enc_alg);

EP_STAT			ep_crypto_key_compat(
				const EP_CRYPTO_KEY *pubkey,
				const EP_CRYPTO_KEY *seckey);
int			ep_crypto_keyform_byname(
				const char *fmt);
int			ep_crypto_keytype_fromkey(
				EP_CRYPTO_KEY *key);
int			ep_crypto_keytype_byname(
				const char *alg_name);
const char		*ep_crypto_keytype_name(
				int keytype);


/*
**  Message Digests
**
**	Note that the cryptographic context (EP_CRYPTO_MD) is
**	also used for signing and verification.
*/

# define EP_CRYPTO_MD		EVP_MD_CTX
# define EP_CRYPTO_MD_MAXSIZE	EVP_MAX_MD_SIZE

// digest algorithms (no more than 4 bits)
# define EP_CRYPTO_MD_NULL	0
# define EP_CRYPTO_MD_SHA1	1
# define EP_CRYPTO_MD_SHA224	2
# define EP_CRYPTO_MD_SHA256	3
# define EP_CRYPTO_MD_SHA384	4
# define EP_CRYPTO_MD_SHA512	5
# define EP_CRYPTO_MD_RIPEMD160	6

EP_STAT			ep_crypto_md_new(
				int md_alg_id,
				EP_CRYPTO_MD **pmd);
EP_CRYPTO_MD		*ep_crypto_md_clone(
				EP_CRYPTO_MD *base_md);
void			ep_crypto_md_free(
				EP_CRYPTO_MD *md);

EP_STAT			ep_crypto_md_update(
				EP_CRYPTO_MD *md,
				const void *data,
				size_t dsize);
EP_STAT			ep_crypto_md_final(
				EP_CRYPTO_MD *md,
				void *dbuf,
				size_t *dbufsize);

int			ep_crypto_md_type(
				EP_CRYPTO_MD *md);
int			ep_crypto_md_alg_byname(
				const char *algname);
const char		*ep_crypto_md_alg_name(
				int md_alg);
int			ep_crypto_md_len(
				int md_alg_id);

void			ep_crypto_md_sha256(
				const void *data,
				size_t glen,
				uint8_t *out);

// private
const EVP_MD		*_ep_crypto_md_getalg_byid(
				int md_alg_id);

/*
**  Signing and Verification
**
**	Note that these operate on EP_CRYPTO_MD structures.
*/

# define EP_CRYPTO_MAX_SIG	(16384 / 8)	// in bytes

EP_STAT			ep_crypto_sign_new(
				EP_CRYPTO_KEY *skey,
				int md_alg_id,
				EP_CRYPTO_MD **pmd);
void			ep_crypto_sign_free(
				EP_CRYPTO_MD *md);
EP_STAT			ep_crypto_sign_update(
				EP_CRYPTO_MD *md,
				void *dbuf,
				size_t dbufsize);
EP_STAT			ep_crypto_sign_final(
				EP_CRYPTO_MD *md,
				void *sbuf,
				size_t *sbufsize);
void			ep_crypto_sign_free(
				EP_CRYPTO_MD *md);

EP_STAT			ep_crypto_vrfy_new(
				EP_CRYPTO_KEY *pkey,
				int md_alg_id,
				EP_CRYPTO_MD **pmd);
void			ep_crypto_vrfy_free(
				EP_CRYPTO_MD *md);
EP_STAT			ep_crypto_vrfy_update(
				EP_CRYPTO_MD *md,
				void *dbuf,
				size_t dbufsize);
EP_STAT			ep_crypto_vrfy_final(
				EP_CRYPTO_MD *md,
				void *obuf,
				size_t obufsize);
void			ep_crypto_vrfy_free(
				EP_CRYPTO_MD *md);


/*
**  Symmetric Ciphers
**	Algorithm definitions are above
*/

typedef struct ep_crypto_cipher_ctx	EP_CRYPTO_CIPHER_CTX;

#define EP_CRYPTO_MODE_CBC	0x100		// Cipher Block Chaining
#define EP_CRYPTO_MODE_CFB	0x200		// Cipher Feedback
#define EP_CRYPTO_MODE_OFB	0x300		// Output Feedback

#define EP_CRYPTO_MODE_MASK	0xf00		// mask for cipher mode

/*
**	The cipher is set to encrypt or decrypt when the context
**	is created.
**
**	ep_crypto_cipher_crypt is just shorthand for a single
**	call to ep_crypto_cipher_update followed by a single
**	call to ep_crypto_cipher_final.  Final pads out any
**	remaining block and returns that data.
*/

EP_CRYPTO_CIPHER_CTX	*ep_crypto_cipher_new(
				uint32_t ciphertype,	// mode + keytype & len
				uint8_t *key,		// the key
				uint8_t *iv,		// initialization vector
				bool enc);		// true => encrypt
void			ep_crypto_cipher_free(
				EP_CRYPTO_CIPHER_CTX *cipher);

EP_STAT			ep_crypto_cipher_crypt(
				EP_CRYPTO_CIPHER_CTX *cipher,
				void *in,		// input data
				size_t inlen,		// input length
				void *out,		// output buffer
				size_t outlen);		// output buf size
EP_STAT			ep_crypto_cipher_update(
				EP_CRYPTO_CIPHER_CTX *cipher,
				void *in,		// input data
				size_t inlen,		// input length
				void *out,		// output buffer
				size_t outlen);		// output buf size
EP_STAT			ep_crypto_cipher_final(
				EP_CRYPTO_CIPHER_CTX *cipher,
				void *out,		// output buffer
				size_t outlen);		// output buf size


/*
**  Error Handling (private)
*/

EP_STAT			_ep_crypto_error(
				EP_STAT estat_default,
				const char *msg, ...);


/*
**  Back compatilibity with old versions of OpenSSL
**	"EVP_MD_CTX_create() and EVP_MD_CTX_destroy() were renamed to
**	 EVP_MD_CTX_new() and EVP_MD_CTX_free() in OpenSSL 1.1."
**
**	LibreSSL uses the old names, and OpenSSL has compatibility macros,
**	so stick with old names.
*/

//#if OPENSSL_VERSION_NUMBER < 0x010100000
//# define EVP_MD_CTX_new		EVP_MD_CTX_create
//# define EVP_MD_CTX_free	EVP_MD_CTX_destroy
//#endif

__END_DECLS
//------------------------------------------------------------------------------
//	Added for C++ compability
}
//------------------------------------------------------------------------------

#endif // _EP_CRYPTO_H_
