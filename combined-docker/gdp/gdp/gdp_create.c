/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**  An API to create new GDP Objects (GOBs or logs) with metadata.
**
**		This works by having a "creation info" data structure that
**		defines the parameters of creation.  That gets modified
**		and then used to do the final population of the metadata
**		before creating a log.
**
**		This is a bit odd because it interacts with both the system
**		environment (to read and store keys) and possibly the
**		interactive user (to encrypt a new secret key or unlock
**		an existing secret key).
**
**	----- BEGIN LICENSE BLOCK -----
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
**	----- END LICENSE BLOCK -----
*/

#include "gdp.h"
#include "gdp_priv.h"
#include "gdp_md.h"

#include <ep/ep_dbg.h>
#include <ep/ep_file_search.h>
#include <ep/ep_prflags.h>
#include <ep/ep_string.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <sys/stat.h>

#if _EP_CRYPTO_INCLUDE_RSA
# include <openssl/rsa.h>
#endif
#if _EP_CRYPTO_INCLUDE_DSA
# include <openssl/dsa.h>
#endif
#if _EP_CRYPTO_INCLUDE_DH
# include <openssl/dh.h>
#endif


#define MINMDS		4		// minimum number of metadata entries (must be > 3)
#ifndef GDP_MIN_KEY_BITS
# define GDP_MIN_KEY_BITS 1024		// pretty RSA-specific
#endif

static EP_DBG	Dbg = EP_DBG_INIT("gdp.create", "GDP object creation");


/*
**  Creation information.
*/

// key + digest algorithm
struct gci_key
{
	EP_CRYPTO_KEY		*key;				// the actual key
	int					dig_alg_id;			// digest algorithm internal id
	int					key_enc_alg_id;		// on-disk encryption algorithm id
	char				*key_enc_pw;		// on-disk encryption key
	int					flags;				// see below
};

// flags for key + digest
#define GCIKF_SET		0x0001			// value has been set
#define GCIKF_USER		0x0002			// user owns key memory

EP_PRFLAGS_DESC		GciFlags[] =
{
	{ GCIKF_SET,			GCIKF_SET,			"SET"			},
	{ GCIKF_USER,			GCIKF_USER,			"USER"			},
	{ 0,					0,					NULL			}
};

// GCI structure
struct gdp_create_info
{
	uint32_t			flags;			// see below
	char				*creator;		// name of log creator (user@domain)
	struct gci_key		owner_key;		// key for admin operations
	struct gci_key		writer_key;		// key for writing data
	gdp_name_t			service_name;	// the name of the creation service
	gdp_md_t			*md;			// metadata list
};

// flags


/*
**  Create and free the data structure.
*/

gdp_create_info_t *
gdp_create_info_new(void)
{
	gdp_create_info_t *gci = ep_mem_zalloc(sizeof *gci);
	return gci;
}


void
gdp_create_info_free(gdp_create_info_t **pgci)
{
	gdp_create_info_t *gci = *pgci;
	if (gci == NULL)
		return;

	// free various malloced fields
	if (gci->creator != NULL)
		ep_mem_free(gci->creator);

	// free keys, being sure not to leave private info in memory
	if (!EP_UT_BITSET(GCIKF_USER, gci->writer_key.flags) &&
			gci->writer_key.key != NULL &&
			gci->writer_key.key != gci->owner_key.key)
		ep_crypto_key_free(gci->writer_key.key);
	if (gci->writer_key.key_enc_pw != NULL)
	{
		memset(gci->writer_key.key_enc_pw, 0, strlen(gci->writer_key.key_enc_pw));
		ep_mem_free(gci->writer_key.key_enc_pw);
	}
	if (!EP_UT_BITSET(GCIKF_USER, gci->owner_key.flags) &&
			gci->owner_key.key != NULL)
		ep_crypto_key_free(gci->owner_key.key);
	if (gci->owner_key.key_enc_pw != NULL)
	{
		memset(gci->owner_key.key_enc_pw, 0, strlen(gci->owner_key.key_enc_pw));
		ep_mem_free(gci->owner_key.key_enc_pw);
	}

	// finally free the extra metadata and the object itself
	if (gci->md != NULL)
		gdp_md_free(gci->md);
	ep_mem_free(gci);
	*pgci = NULL;
}


/*
**  Key management
*/

/*
**  Create new {owner | writer} key.
**		Given a hash (digest) algorithm and key information,
**		create the keypair and store it in the create_info.
*/

static EP_STAT
gci_new_key(
		const char *where,
		struct gci_key *gcik,
		const char *dig_alg_name,
		const char *key_alg_name,
		int key_bits,
		const char *key_curve,
		const char *key_enc_alg_name)
{
	EP_STAT estat = EP_STAT_OK;

	ep_dbg_cprintf(Dbg, 16,
			"gci_new_key(%s): dig %s key %s bits %d curve %s enc %s\n",
			where, dig_alg_name, key_alg_name, key_bits, key_curve,
			key_enc_alg_name);

	// message digest (hash) algorithm
	if (dig_alg_name == NULL || strcmp(dig_alg_name, "def") == 0)
	{
		dig_alg_name = ep_adm_getstrparam("swarm.gdp.crypto.digest.alg",
								"sha256");
	}
	gcik->dig_alg_id = ep_crypto_md_alg_byname(dig_alg_name);
	if (gcik->dig_alg_id < 0)
	{
		ep_dbg_cprintf(Dbg, 1,
				"gci_new_key(%s): unknown digest hash algorithm %s\n",
				where, dig_alg_name);
		return EP_STAT_CRYPTO_HASHALG;
	}

	// asymmetric key type (rsa, ec, etc.)
	if (key_alg_name == NULL || strcmp(key_alg_name, "def") == 0)
	{
		key_alg_name = ep_adm_getstrparam("swarm.gdp.crypto.sign.alg", "ec");
	}
	int key_alg_id = ep_crypto_keytype_byname(key_alg_name);
	int key_exponent = 0;				// used by RSA only
	switch (key_alg_id)
	{
	case EP_CRYPTO_KEYTYPE_NULL:
		// NULL crypto specified
		goto done;

	case EP_CRYPTO_KEYTYPE_RSA:
		if (key_bits <= 0)
			key_bits = ep_adm_getintparam("swarm.gdp.crypto.rsa.keylen", 2048);
		key_exponent = ep_adm_getintparam("swarm.gdp.crypto.rsa.keyexp", 3);
		break;

	case EP_CRYPTO_KEYTYPE_DSA:
		if (key_bits <= 0)
			key_bits = ep_adm_getintparam("swarm.gdp.crypto.dsa.keylen", 2048);
		break;

	case EP_CRYPTO_KEYTYPE_EC:
		if (key_curve == NULL)
			key_curve = ep_adm_getstrparam("swarm.gdp.crypto.ec.curve", NULL);
		break;

	default:
		ep_dbg_cprintf(Dbg, 1, "unknown key type %s\n", key_alg_name);
		return EP_STAT_CRYPTO_KEYTYPE;
	}

	if (key_bits < GDP_MIN_KEY_BITS && key_alg_id != EP_CRYPTO_KEYTYPE_EC)
	{
		ep_dbg_cprintf(Dbg, 1, "Insecure key length %d; %d min\n",
				key_bits, GDP_MIN_KEY_BITS);
		estat = EP_STAT_CRYPTO_TOOSMALL;
		goto fail0;
	}

	// secret key on-disk crypto algorithm
	if (key_enc_alg_name == NULL || strcmp(key_enc_alg_name, "def") == 0)
	{
		key_enc_alg_name = ep_adm_getstrparam("swarm.gdp.crypto.keyenc.alg",
								"aes192");
	}
	gcik->key_enc_alg_id = ep_crypto_keyenc_byname(key_enc_alg_name);
	if (gcik->key_enc_alg_id < 0)
	{
		ep_dbg_cprintf(Dbg, 1,
				"gci_new_key(%s): unknown secret key encryption algorithm %s\n",
				where, key_enc_alg_name);
		estat = EP_STAT_CRYPTO_CIPHER;
	}

	gcik->key = ep_crypto_key_create(key_alg_id, key_bits, key_exponent, key_curve);
	if (gcik->key == NULL)
	{
		ep_dbg_cprintf(Dbg, 1, "Could not create new key");
		estat = EP_STAT_CRYPTO_KEYCREATE;
		goto fail0;
	}

	if (ep_dbg_test(Dbg, 32))
	{
		ep_dbg_printf("gci_new_key: created new secret key:\n");
		ep_crypto_key_print(gcik->key, ep_dbg_getfile(),
					ep_dbg_test(Dbg, 42) ? EP_CRYPTO_F_SECRET : EP_CRYPTO_F_PUBLIC);
	}

done:
	gcik->flags |= GCIKF_SET;
fail0:
	return estat;
}


EP_STAT
gdp_create_info_new_owner_key(
		gdp_create_info_t *gci,
		const char *dig_alg_name,
		const char *key_alg_name,
		int key_bits,
		const char *key_curve,
		const char *key_enc_alg_name)
{
	EP_STAT estat;

	estat = gci_new_key("owner", &gci->owner_key,
					dig_alg_name, key_alg_name, key_bits, key_curve,
					key_enc_alg_name);
	return estat;
}

EP_STAT
gdp_create_info_new_writer_key(
		gdp_create_info_t *gci,
		const char *dig_alg_name,
		const char *key_alg_name,
		int key_bits,
		const char *key_curve,
		const char *key_enc_alg_name)
{
	EP_STAT estat;

	estat = gci_new_key("writer", &gci->writer_key,
					dig_alg_name, key_alg_name, key_bits, key_curve,
					key_enc_alg_name);
	return estat;
}


/*
**  Set { owner | writer } key to externally supplied value.
**
**		The hash (digest) algorithm has to be specified here, since
**		it isn't part of the key.
*/

static EP_STAT
gci_set_key(
		const char *where,
		struct gci_key *gcik,
		const char *dig_alg_name,
		EP_CRYPTO_KEY *key)
{
	EP_STAT estat = EP_STAT_OK;

	// message digest (hash) algorithm
	if (dig_alg_name == NULL || strcmp(dig_alg_name, "def") == 0)
	{
		dig_alg_name = ep_adm_getstrparam("swarm.gdp.crypto.digest.alg",
								"sha256");
	}
	gcik->dig_alg_id = ep_crypto_md_alg_byname(dig_alg_name);
	if (gcik->dig_alg_id < 0)
	{
		ep_dbg_cprintf(Dbg, 1,
				"gci_set_key(%s): unknown digest hash algorithm %s\n",
				where, dig_alg_name);
		return EP_STAT_CRYPTO_HASHALG;
	}
	if (gcik->key != NULL && !EP_UT_BITSET(GCIKF_USER, gcik->flags))
		ep_crypto_key_free(gcik->key);
	gcik->key = key;
	gcik->flags |= GCIKF_SET | GCIKF_USER;

	return estat;
}

EP_STAT
gdp_create_info_set_owner_key(
					gdp_create_info_t *gci,
					EP_CRYPTO_KEY *key,
					const char *dig_alg_name)
{
	EP_STAT estat;

	estat = gci_set_key("owner", &gci->owner_key, dig_alg_name, key);
	return estat;
}


EP_STAT
gdp_create_info_set_writer_key(
					gdp_create_info_t *gci,
					EP_CRYPTO_KEY *key,
					const char *dig_alg_name)
{
	EP_STAT estat;

	estat = gci_set_key("writer", &gci->writer_key, dig_alg_name, key);
	return estat;
}


/*
**  Find the pathname for a key file.
*/

static char *
gci_key_file_name(const char *key_dir_path,
				const gdp_name_t gname,
				const char *key_scope,
				const char *key_suffix)
{
	EP_STAT estat = EP_STAT_OK;

	// find the directory part of the file name by searching
	if (key_dir_path == NULL)
		key_dir_path = ep_adm_getstrparam("swarm.gdp.crypto.key.dir", "KEYS");
	char key_dir_name[PATH_MAX];
	estat = ep_file_search(key_dir_path, NULL, sizeof key_dir_name, key_dir_name);
	if (!EP_STAT_ISOK(estat))
		strcpy(key_dir_name, ".");

	// now get the rest of the file name
	//XXX should this use the "/_xx/" path like GOB data files?
	gdp_pname_t pbuf;
	gdp_printable_name(gname, pbuf);
	size_t len = strlen(key_dir_name) + sizeof pbuf + 6;
	if (key_scope != NULL)
		len += strlen(key_scope) + 1;
	if (key_suffix == NULL)
		key_suffix = "";
	len += strlen(key_suffix);
	char *key_file_name = (char *) ep_mem_malloc(len);
	if (key_scope != NULL)
		snprintf(key_file_name, len, "%s/%s-%s.pem%s",
				key_dir_name, pbuf, key_scope, key_suffix);
	else
		snprintf(key_file_name, len, "%s/%s.pem%s",
				key_dir_name, pbuf, key_suffix);
	return key_file_name;
}


/*
**  Write key to stable storage.
**
**		`key_dir_path` is the user-specified directory search path,
**			if any.  If not specified, a system default will be used.
**		`gname` is the name of the log to which this key applies.
**		`key_scope` is the semantics of the key ("owner", "writer").
*/

static EP_STAT
gci_save_key(
		struct gci_key *gcik,
		const char *key_dir_path,
		const gdp_name_t gname,
		const char *key_scope)
{
	EP_STAT estat = EP_STAT_OK;

	if (!EP_ASSERT(gcik != NULL))
		return EP_STAT_ASSERT_ABORT;

	gdp_pname_t pname;
	ep_dbg_cprintf(Dbg, 16,
			"gci_save_key(%s) dir %s key %p\n    name %s\n",
			key_scope, key_dir_path, gcik,
			gdp_printable_name(gname, pname));

	if (gcik->key != NULL)
	{
		char *key_file_name = gci_key_file_name(key_dir_path, gname, key_scope, NULL);
		int fd;
		FILE *fp;
		int oflags = O_WRONLY | O_CREAT | O_TRUNC;
		int omode = 0400;

		omode = ep_adm_getintparam("swarm.gdp.crypto.key.mode", 0400);
		if (ep_adm_getboolparam("swarm.gdp.crypto.key.exclusive", true))
				oflags |= O_EXCL;

		if ((fd = open(key_file_name, oflags, omode)) < 0 ||
				(fp = fdopen(fd, "w")) == NULL)
		{
			ep_dbg_cprintf(Dbg, 1, "gci_save_key: cannot create %s: %s\n",
					key_file_name, strerror(errno));
			estat = ep_stat_from_errno(errno);
//			gcik->flags |= GCIKF_USER;		// avoid later removal of this file
			goto fail0;
		}

		estat = ep_crypto_key_write_fp(gcik->key, fp,
						EP_CRYPTO_KEYFORM_PEM,
						gcik->key_enc_alg_id, gcik->key_enc_pw,
						EP_CRYPTO_F_SECRET);

		// TODO: should really clear the fp buffer memory here to
		//		 avoid exposing any secret key information after free
		fclose(fp);

		if (!EP_STAT_ISOK(estat))
		{
			char ebuf[200];

			// remove the zero length key file
			unlink(key_file_name);

			ep_dbg_cprintf(Dbg, 1,
					"gci_save_key: ouldn't write secret key to %s: %s\n",
					key_file_name, ep_stat_tostr(estat, ebuf, sizeof ebuf));
		}

fail0:
		ep_mem_free(key_file_name);
	}
	return estat;
}


static EP_STAT
gci_remove_key(
		struct gci_key *gcik,
		const char *key_dir_path,
		const gdp_name_t gname,
		const char *key_scope)
{
	EP_STAT estat = EP_STAT_OK;

	if (!EP_ASSERT(gcik != NULL && gcik->key != NULL))
		return EP_STAT_ASSERT_ABORT;

	char *key_file_name =
					gci_key_file_name(key_dir_path, gname, key_scope, NULL);
#if GDP_CF_CREATE_SAVE_BACKUP_KEY_FILE
	// if you keep backups, be sure to remove them periodically, e.g.,
	//		find /etc/gdp/keys -name \*.BAK -atime +2 -exec rm {}
	char *backup_file_name =
					gci_key_file_name(key_dir_path, gname, key_scope, ".BAK");
	rename(key_file_name, backup_file_name);
	ep_mem_free(backup_file_name);
#else
	unlink(key_file_name);
#endif
	ep_mem_free(key_file_name);
	return estat;
}


/*
**  Manipulate other parameters (less common).
*/

EP_STAT
gdp_create_info_set_creator(
		gdp_create_info_t *gci,
		const char *user,
		const char *domain)
{
	if (user == NULL)
		user = "";
	if (domain == NULL)
		domain = "";
	if (*user == '\0' && strchr(domain, '@') != NULL)
		gci->creator = ep_mem_strdup(domain);
	else if (*domain == '\0' && strchr(user, '@') != NULL)
		gci->creator = ep_mem_strdup(user);
	else
	{
		char userbuf[40];
		//  (Linux sets HOST_NAME_MAX too low, so we use a magic constant)
		char fqdnbuf[1025];

		if (*user == '\0')
		{
			// get user name from system (passwd file)
			struct passwd *pw = getpwuid(getuid());

			if (pw != NULL)
			{
				user = pw->pw_name;
			}
			else
			{
				snprintf(userbuf, sizeof userbuf, "%d", getuid());
				user = userbuf;
			}
		}

		if (*domain == '\0')
		{
			// get fully qualified domain name from system
			// gethostname doesn't guarantee null termination
			fqdnbuf[sizeof fqdnbuf - 1] = '\0';
			if (gethostname(fqdnbuf, sizeof fqdnbuf - 1) != 0)
			{
				ep_dbg_cprintf(Dbg, 1, "Cannot find current host name\n");
				strlcpy(fqdnbuf, "localhost", sizeof fqdnbuf);
			}

			if (strchr(fqdnbuf, '.') == NULL)
			{
				// need to tack on a domain name
				struct addrinfo hints, *ai;
				int i;

				memset(&hints, 0, sizeof hints);
				hints.ai_family = AF_UNSPEC;
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_flags = AI_CANONNAME;

				if ((i = getaddrinfo(fqdnbuf, NULL, &hints, &ai)) != 0)
				{
					ep_dbg_cprintf(Dbg, 1, "Cannot find DNS domain name: %s\n",
							gai_strerror(i));
				}
				else
				{
					strlcpy(fqdnbuf, ai->ai_canonname, sizeof fqdnbuf);
				}
			}
			domain = fqdnbuf;
		}
		int vlen = strlen(user) + strlen(domain) + 2;
		char vbuf[vlen];
		snprintf(vbuf, vlen, "%s@%s", user, domain);
		gci->creator = ep_mem_strdup(vbuf);
	}
	ep_dbg_cprintf(Dbg, 24, "gdp_create_info_set_creator(%s)\n", gci->creator);
	return EP_STAT_OK;
}

EP_STAT
gdp_create_info_set_expiration(
		gdp_create_info_t *gci,
		uint64_t expiration)
{
	// unclear how to implement this
	return GDP_STAT_NOT_IMPLEMENTED;
}

EP_STAT
gdp_create_info_set_creation_service(
		gdp_create_info_t *gci,
		const char *x_svc_name)
{
	EP_STAT estat;

	estat = gdp_name_parse(x_svc_name, gci->service_name, NULL);
	if (EP_STAT_ISOK(estat))
		return estat;

	char ebuf[100];
	ep_dbg_cprintf(Dbg, 1,
			"gdp_create_info_set_creation_service(%s): %s\n",
			x_svc_name, ep_stat_tostr(estat, ebuf, sizeof ebuf));
	return estat;
}


/*
**  Escape allowing app to add arbitrary metadata.
*/

EP_STAT
gdp_create_info_add_metadata(
		gdp_create_info_t *gci,
		uint32_t md_name,
		size_t md_len,
		const void *md_val)
{
	EP_STAT estat = EP_STAT_OK;

	if (gci->md == NULL)
		gci->md = gdp_md_new(0);
	estat = gdp_md_add(gci->md, md_name, md_len, md_val);
	return estat;
}


/*
**  Enter a key into the metadata, creating it if necessary
*/

static EP_STAT
gci_enter_key(struct gci_key *gcik,
		const char *where,
		uint32_t md_name,
		gdp_md_t *gmd)
{
	EP_STAT estat = EP_STAT_OK;
	struct gci_key gcikbuf;

	if (ep_dbg_test(Dbg, 16))
	{
		ep_dbg_printf("gci_enter_key(%s): gcik %p", where, gcik);
		if (gcik != NULL)
		{
			ep_dbg_printf(" flags ");
			ep_prflags(gcik->flags, GciFlags, NULL);
		}
		ep_dbg_printf("\n");

	}
	if (!EP_UT_BITSET(GCIKF_SET, gcik->flags))
	{
		// hasn't been set, so create the new key using defaults.
		// we use a buffer here so that gcik doesn't get modified.
		memset(&gcikbuf, 0, sizeof gcikbuf);
		gcik = &gcikbuf;

		// create new key with defaults
		estat = gci_new_key(where, gcik, NULL, NULL, 0, NULL, NULL);
		gcik->flags |= GCIKF_SET;
	}

	if (gcik->key != NULL)
	{
		// add the public key (if it exists) to the metadata
		uint8_t der_buf[EP_CRYPTO_MAX_DER + 4];
		uint8_t *derp = der_buf + 4;
		int key_alg_id = ep_crypto_keytype_fromkey(gcik->key);
		int key_bits = 0;

#if OPENSSL_VERSION_NUMBER >= 0x10100000L	// 1.1.0 dev
		switch (key_alg_id)
		{
#if _EP_CRYPTO_INCLUDE_RSA
		  case EP_CRYPTO_KEYTYPE_RSA:
			  key_bits = RSA_bits(EVP_PKEY_get1_RSA(gcik->key));
			  break;
#endif

#if _EP_CRYPTO_INCLUDE_DSA
		  case EP_CRYPTO_KEYTYPE_DSA:
			  key_bits = DSA_bits(EVP_PKEY_get1_DSA(gcik->key));
			  break;
#endif

#if _EP_CRYPTO_INCLUDE_DH
		  case EP_CRYPTO_KEYTYPE_DH:
			  key_bits = DH_bits(EVP_PKEY_get1_DH(gcik->key));
			  break;
#endif
		}
#endif

		der_buf[0] = gcik->dig_alg_id;
		der_buf[1] = key_alg_id;
		der_buf[2] = (key_bits >> 8) & 0xff;
		der_buf[3] = key_bits & 0xff;
		estat = ep_crypto_key_write_mem(gcik->key, derp, EP_CRYPTO_MAX_DER,
						EP_CRYPTO_KEYFORM_DER, EP_CRYPTO_SYMKEY_NONE, NULL,
						EP_CRYPTO_F_PUBLIC);
		if (!EP_STAT_ISOK(estat))
		{
			char ebuf[100];
			ep_dbg_cprintf(Dbg, 1,
					"Could not create DER format public key: %s\n",
					ep_stat_tostr(estat, ebuf, sizeof ebuf));
			goto fail0;
		}

		estat = gdp_md_add(gmd, md_name, EP_STAT_TO_INT(estat) + 4, der_buf);
		EP_STAT_CHECK(estat, goto fail0);
	}

fail0:
	if (gcik == &gcikbuf && gcik->key != NULL)
		ep_crypto_key_free(gcik->key);
	return estat;
}


/*
**  Actually create a new log.
**
**	This is a multistep process:
**		+ Create the metadata if it isn't already there.
**		+ Create the owner key if it hasn't been specified.
**		+ Copy the owner key to the writer key if necessary.
**		+ Populate the metadata with info from create_info,
**		  utilizing defaults as necessary.
**		+ Add the human_name to the metadata if specified.
**		+ Add a nonce to the metadata.
**		+ Hash the metadata to determine the new log name.
**		+ Store secret keys on disk indexed by the new name.
**		  XXX should this be the hash of the public key? XXX
**		+ Send the actual on-the-wire create command.
**		+ If the command failed, clean up unused keys.
**		+ If the command succeeded, register the human_name.
*/

EP_STAT
gdp_gin_create(
		gdp_create_info_t *gci,
		const char *human_name,
		gdp_gin_t **pgin)
{
	EP_STAT estat = EP_STAT_OK;
	gdp_md_t *gmd = NULL;

	ep_dbg_cprintf(Dbg, 19, "gdp_gin_create(%s)\n",
			human_name == NULL ? "~no name~" : human_name);

	/*
	**  Verify that the human_name is not a Chimera
	*/

	if (human_name != NULL)
	{
		if (human_name[0] == '\0')
		{
			// valid, but same as nothing
			human_name = NULL;
		}
		else
		{
			gdp_name_t iname;
			if (EP_STAT_ISOK(gdp_internal_name(human_name, iname)))
			{
				// this is a valid base64-encoded name: confusion reigns
				estat = GDP_STAT_GDP_NAME_INVALID;
				goto fail0;
			}
		}
	}

	/*
	**  Fill in some invariant (for this invocation) metadata.
	*/

	if (gci->creator == NULL)
		gdp_create_info_set_creator(gci, NULL, NULL);

	/*
	**  Make sure we have some metadata, even if no create_info.
	**
	**		We clone the gdp_create_info here since it could be used to
	**		create more than one log.
	*/

	if (gci != NULL && gci->md != NULL)
		gmd = _gdp_md_clone(gci->md);
	if (gmd == NULL)
		gmd = gdp_md_new(MINMDS);

	/*
	**  Set some fixed things
	*/

	// external name (or description)
	if (human_name != NULL)
	{
		estat = gdp_md_add(gmd, GDP_MD_XID,
						strlen(human_name), human_name);
		EP_STAT_CHECK(estat, goto fail0);
	}

	// creation time
	{
		EP_TIME_SPEC tv;
		char timestring[40];

		ep_time_now(&tv);
		ep_time_format(&tv, timestring, sizeof timestring, EP_TIME_FMT_DEFAULT);
		ep_dbg_cprintf(Dbg, 24, "gdp_gin_create: ctime=%s\n", timestring);
		estat = gdp_md_add(gmd, GDP_MD_CTIME, strlen(timestring), timestring);
		EP_STAT_CHECK(estat, goto fail0);
	}

	// nonce
	{
		uint8_t nonce[16];
		ep_crypto_random_buf(nonce, sizeof nonce);
		estat = gdp_md_add(gmd, GDP_MD_NONCE, sizeof nonce, nonce);
		EP_STAT_CHECK(estat, goto fail0);
	}

	/*
	**  Populate the metadata with info from create_info.
	*/

	// creator
	if (gci->creator != NULL)
	{
		estat = gdp_md_add(gmd, GDP_MD_CREATOR,
						strlen(gci->creator), gci->creator);
	}

	// expiration date
	//TODO
#if 0
	if (gci->expiration > 0)
	{
		do something here;
	}
#endif

	/*
	**  Create keys if needed and populate metadata.
	**
	**		If the owner key has not been set, create one.
	**		If the writer key has not been set, use the owner key.
	*/

	gci_enter_key(&gci->owner_key, "owner", GDP_MD_OWNERPUBKEY, gmd);
	if (EP_UT_BITSET(GCIKF_SET, gci->writer_key.flags))
		gci_enter_key(&gci->writer_key, "writer", GDP_MD_WRITERPUBKEY, gmd);

	/*
	**  Now have all the metadata: we can serialize and create the name.
	*/

	gdp_name_t gname;
	estat = _gdp_md_to_gdpname(gmd, &gname, NULL);
	EP_STAT_CHECK(estat, goto fail1);
	if (ep_dbg_test(Dbg, 19))
	{
		gdp_pname_t pname;
		ep_dbg_printf("gdp_gin_create: using name %s\n",
				gdp_printable_name(gname, pname));
	}

	/*
	**  Store the secret key(s) on disk.
	**  If the user specified a key it will already be stored on
	**  disk, but probably under a different name than we can find.
	**	We might be able to use a link under some circumstances, but
	**	not necessarily all, so we pessimise.
	*/

	if (EP_UT_BITSET(GCIKF_SET, gci->owner_key.flags))
	{
		estat = gci_save_key(&gci->owner_key, NULL, gname, "owner");
		EP_STAT_CHECK(estat, goto fail1);
	}
	if (EP_UT_BITSET(GCIKF_SET, gci->writer_key.flags))
	{
		estat = gci_save_key(&gci->writer_key, NULL, gname, "writer");
		EP_STAT_CHECK(estat, goto fail1);
	}

	/*
	**  Send the actual on-the-wire create command.
	*/

	gdp_gob_t *gob;
	estat = _gdp_gob_create(gmd, gci->service_name, &gob);
	EP_STAT_CHECK(estat, goto fail1);

	/*
	**  Cleanup and complete.
	*/

	*pgin = _gdp_gin_new(gob);

fail1:
	if (!EP_STAT_ISOK(estat))
	{
		// if we created the key(s), remove them
		if (gci->owner_key.key != NULL &&
				EP_UT_BITSET(GCIKF_SET, gci->owner_key.flags))
			gci_remove_key(&gci->owner_key, NULL, gname, "owner");
		if (gci->writer_key.key != NULL &&
				EP_UT_BITSET(GCIKF_SET, gci->writer_key.flags))
			gci_remove_key(&gci->writer_key, NULL, gname, "writer");
	}
fail0:
	return estat;
}
