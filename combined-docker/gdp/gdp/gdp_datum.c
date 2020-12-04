/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**  Message management
**
**		Messages contain the header and data information for a single
**		message.
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

#include <ep/ep.h>
#include <ep/ep_dbg.h>
#include <ep/ep_hexdump.h>
#include <ep/ep_prflags.h>
#include <ep/ep_string.h>
#include <ep/ep_thr.h>

#include "gdp.h"
#include "gdp_md.h"
#include "gdp_priv.h"

#include <string.h>

static EP_DBG	Dbg = EP_DBG_INIT("gdp.datum", "GDP datum processing");

static gdp_datum_t		*DatumFreeList;
static EP_THR_MUTEX		DatumFreeListMutex EP_THR_MUTEX_INITIALIZER2(GDP_MUTEX_LORDER_LEAF);


/*
**  Create a new datum.
**		The datum is returned unlocked with a data buffer.
*/

gdp_datum_t *
gdp_datum_new(void)
{
	gdp_datum_t *datum;

	// get a message off the free list, if any
	ep_thr_mutex_lock(&DatumFreeListMutex);
	if ((datum = DatumFreeList) != NULL)
	{
		DatumFreeList = datum->next;
	}
	ep_thr_mutex_unlock(&DatumFreeListMutex);

	if (datum == NULL)
	{
		// nothing on the free list; allocate anew
		datum = (gdp_datum_t *) ep_mem_zalloc(sizeof *datum);
		ep_thr_mutex_init(&datum->mutex, EP_THR_MUTEX_DEFAULT);
		ep_thr_mutex_setorder(&datum->mutex, GDP_MUTEX_LORDER_DATUM);
	}
	datum->next = NULL;

	EP_ASSERT(!EP_UT_BITSET(GDP_DF_INUSE, datum->flags));

	// initialize metadata
	gdp_datum_reset(datum);
	ep_dbg_cprintf(Dbg, 48, "gdp_datum_new => %p\n", datum);
	VALGRIND_HG_CLEAN_MEMORY(datum, sizeof *datum);
	return datum;
}


void
gdp_datum_free(gdp_datum_t *datum)
{
	ep_dbg_cprintf(Dbg, 48, "gdp_datum_free(%p)\n", datum);

	// sanity
	if (datum == NULL)
		return;
	EP_ASSERT_ELSE(EP_UT_BITSET(GDP_DF_INUSE, datum->flags), return);
	datum->flags = 0;

	if (datum->dbuf != NULL)
	{
		size_t ndrain = gdp_buf_getlength(datum->dbuf);
		ep_dbg_cprintf(Dbg, 50, "  ... draining %zd bytes\n", ndrain);
		if (ndrain > 0)
			gdp_buf_drain(datum->dbuf, ndrain);
	}
	if (datum->sig != NULL)
	{
		//XXX retain this buffer?
		gdp_sig_free(datum->sig);
		datum->sig = NULL;
	}

	// make sure the datum is unlocked before putting on the free list
	if (ep_thr_mutex_trylock(&datum->mutex) != 0)
	{
		// shouldn't happen
		ep_dbg_cprintf(Dbg, 1, "gdp_datum_free(%p): was locked\n", datum);
	}
	ep_thr_mutex_unlock(&datum->mutex);
#if GDP_DEBUG_NO_FREE_LISTS		// avoid helgrind complaints
	ep_thr_mutex_destroy(&datum->mutex);
	if (datum->dbuf == NULL)
		gdp_buf_free(datum->dbuf);
	ep_mem_free(datum);
#else
	ep_thr_mutex_lock(&DatumFreeListMutex);
	datum->next = DatumFreeList;
	DatumFreeList = datum;
	ep_thr_mutex_unlock(&DatumFreeListMutex);
#endif
}


/*
**  Reset a datum
**
**		This resets the data and signature buffers, but leaves the
**		timestamp and recno since the caller might like to see the
**		updated values.
*/

void
gdp_datum_reset(gdp_datum_t *datum)
{
	if (datum->dbuf == NULL)
		datum->dbuf = gdp_buf_new();
	else
		gdp_buf_reset(datum->dbuf);
	if (datum->sig != NULL)
		gdp_sig_reset(datum->sig);
	datum->recno = GDP_PDU_NO_RECNO;
	datum->flags = GDP_DF_INUSE;
	EP_TIME_INVALIDATE(&datum->ts);
}


gdp_recno_t
gdp_datum_getrecno(const gdp_datum_t *datum)
{
	return datum->recno;
}

void
gdp_datum_getts(const gdp_datum_t *datum, EP_TIME_SPEC *ts)
{
	memcpy(ts, &datum->ts, sizeof *ts);
}

size_t
gdp_datum_getdlen(const gdp_datum_t *datum)
{
	return gdp_buf_getlength(datum->dbuf);
}

gdp_buf_t *
gdp_datum_getbuf(const gdp_datum_t *datum)
{
	return datum->dbuf;
}

gdp_sig_t *
gdp_datum_getsig(const gdp_datum_t *datum)
{
	return datum->sig;
}


/*
**	GDP_DATUM_PRINT --- print a datum (for debugging)
*/

static EP_PRFLAGS_DESC	DatumFlagsDesc[] =
{
	{ GDP_DF_INUSE,		GDP_DF_INUSE,		"INUSE"			},
	{ GDP_DF_GOODSIG,	GDP_DF_GOODSIG,		"GOODSIG"		},
	{ 0,				0,					NULL			}
};

void
gdp_datum_print(const gdp_datum_t *datum, FILE *fp, uint32_t flags)
{
	unsigned char *d;
	int l;
	bool quiet = EP_UT_BITSET(GDP_DATUM_PRQUIET, flags);
	bool debug = EP_UT_BITSET(GDP_DATUM_PRDEBUG, flags);

	if (quiet && (debug || EP_UT_BITSET(GDP_DATUM_PRMETAONLY, flags)))
		quiet = false;

	if (!EP_ASSERT(fp != NULL))
		return;

	flockfile(fp);
	if (debug)
		fprintf(fp, "datum @ %p: ", datum);
	if (datum == NULL)
	{
		if (!quiet)
			fprintf(fp, "null datum\n");
		goto done;
	}

	if (!quiet)
		fprintf(fp, "recno %" PRIgdp_recno ", ", datum->recno);

	if (datum->dbuf == NULL)
	{
		if (!quiet)
			fprintf(fp, "no data");
		d = NULL;
		l = -1;
	}
	else
	{
		l = gdp_buf_getlength(datum->dbuf);
		if (!quiet)
			fprintf(fp, "len %d", l);
		if (l > 0)
			d = gdp_buf_getptr(datum->dbuf, l);
		else
			d = (unsigned char *) "";
	}

	if (!quiet)
	{
		if (EP_TIME_IS_VALID(&datum->ts))
		{
			fprintf(fp, ", ts ");
			ep_time_print(&datum->ts, fp, EP_TIME_FMT_HUMAN);
		}
		else
		{
			fprintf(fp, ", no timestamp");
		}

		if (debug)
		{
			fprintf(fp, ", flags ");
			ep_prflags(datum->flags, DatumFlagsDesc, fp);
		}
		fprintf(fp, "\n");
	}

	if (!EP_UT_BITSET(GDP_DATUM_PRMETAONLY, flags))
	{
		if (EP_UT_BITSET(GDP_DATUM_PRTEXT, flags))
			fprintf(fp, "%.*s\n", l, d);
		else if (EP_UT_BITSET(GDP_DATUM_PRBINARY, flags))
			fwrite(fp, l, 1, fp);
		else
			ep_hexdump(d, l, fp, EP_HEXDUMP_ASCII, 0);
	}

	if (EP_UT_BITSET(GDP_DATUM_PRSIG, flags))
	{
		if (datum->sig != NULL)
		{
			size_t siglen;
			d = gdp_sig_getptr(datum->sig, &siglen);
			if (siglen > 0)
			{
				fprintf(fp, "  sig\n");
				ep_hexdump(d, siglen, fp, EP_HEXDUMP_HEX, 0);
			}
			else
				fprintf(fp, "  empty sig\n");
		}
		else
			fprintf(fp, "  no sig\n");
	}
done:
	funlockfile(fp);
}


/*
**  Copy contents of one datum into another
*/

void
gdp_datum_copy(gdp_datum_t *to, const gdp_datum_t *from)
{
	to->recno = from->recno;
	to->ts = from->ts;
	to->flags = from->flags;
	if (from->dbuf != NULL)
	{
		if (EP_ASSERT(to->dbuf != NULL))
			gdp_buf_copy(to->dbuf, from->dbuf);
	}
	if (from->sig != NULL)
	{
		if (EP_ASSERT(to->sig != NULL))
			gdp_sig_copy(to->sig, from->sig);
	}
}


/*
**  Duplicate a datum (internal use)
*/

gdp_datum_t *
gdp_datum_dup(const gdp_datum_t *datum)
{
	gdp_datum_t *ndatum;

	ndatum = gdp_datum_new();
	ndatum->recno = datum->recno;
	ndatum->ts = datum->ts;
	gdp_buf_copy(ndatum->dbuf, datum->dbuf);
	if (datum->sig != NULL)
		ndatum->sig = gdp_sig_dup(datum->sig);

	return ndatum;
}


/*
**  Print a datum (for debugging)
*/

void
_gdp_datum_dump(const gdp_datum_t *datum,
			FILE *fp)
{
	if (fp == NULL)
		fp = ep_dbg_getfile();
	gdp_datum_print(datum, fp, GDP_DATUM_PRDEBUG);
}

/*
**  Incorporate a datum into a pre-existing message digest.
**  This can also be used when signing and verifying.
**
**  The message digest must already be initialized, and can already
**  have data included in it.  Generally speaking, this will include at
**  a minimum the name of the log in which this data is stored, which
**  itself will be the hash of the metadata for that log.  This allows
**  this routine to be usable for both computing a basic hash and for
**  signing.
**
**  This routine adds to the existing digest:
**		* record number
**		* timestamp
**		* hash of previous datum
**		* the "proof" (hash back pointers and offsets)
**		* hash of data payload
**
**	If the datum does not yet include the hash of the data payload,
**	that will be computed and cached.
*/

EP_STAT
_gdp_datum_digest(gdp_datum_t *datum, EP_CRYPTO_MD *md)
{
	if (ep_dbg_test(Dbg, 50))
	{
		ep_dbg_printf("_gdp_datum_digest: ");
		_gdp_datum_dump(datum, NULL);
	}

	// check for upstream problems and avoid core dumps
	if (md == NULL)
		return GDP_STAT_CRYPTO_ERROR;

	// now compute H(recno || timestamp || prevHash || proof || H(data))
	// recno
	{
		uint8_t recnobuf[8];		// 64 bits
		uint8_t *pbp = recnobuf;
		PUT64(datum->recno);
		ep_crypto_md_update(md, &recnobuf, sizeof recnobuf);
	}
	// timestamp
	{
		uint8_t tsbuf[16];			// 64 + 32 + 32 bits
		uint8_t *pbp = tsbuf;
		PUT64(datum->ts.tv_sec);
		PUT32(datum->ts.tv_nsec);
		PUT32(*(uint32_t *) &datum->ts.tv_accuracy);
		ep_crypto_md_update(md, tsbuf, sizeof tsbuf);
	}
	// prevhash
	if (datum->prevhash != NULL)
	{
		size_t prevhashlen;
		void *hashbytes = gdp_hash_getptr(datum->prevhash, &prevhashlen);
		ep_crypto_md_update(md, hashbytes, prevhashlen);
	}
	// proof
	//TODO: include proof
	// data hash
	{
		int hashalg = ep_crypto_md_type(md);
		EP_CRYPTO_MD *dmd;
		EP_STAT estat = ep_crypto_md_new(hashalg, &dmd);

		if (!EP_STAT_ISOK(estat))
		{
			char ebuf[100];
			ep_dbg_cprintf(Dbg, 9,
					"_gdp_datum_digest: ep_crypto_md_new(%d) => %s\n",
					hashalg, ep_stat_tostr(estat, ebuf, sizeof ebuf));
			return estat;
		}
		if (datum->dbuf != NULL)
		{
			size_t dlen = gdp_buf_getlength(datum->dbuf);
			ep_crypto_md_update(dmd, gdp_buf_getptr(datum->dbuf, dlen), dlen);
		}

		uint8_t dhash[EP_CRYPTO_MAX_DIGEST];
		size_t dhlen = sizeof dhash;
		ep_crypto_md_final(dmd, &dhash, &dhlen);
		ep_crypto_md_free(dmd);
		ep_crypto_md_update(md, dhash, dhlen);
	}
	return EP_STAT_OK;
}


/*
**  Compute hash of a datum.
**
**		Two versions, one using a GIN, other using a GOB.
**		This is not usable for signing or verification.
*/

gdp_hash_t *
gdp_datum_hash(gdp_datum_t *datum, gdp_gin_t *gin)
{
	return _gdp_datum_hash(datum, gin->gob);
}


gdp_hash_t *
_gdp_datum_hash(gdp_datum_t *datum, gdp_gob_t *gob)
{
	if (gob->vrfy_ctx == NULL)
	{
		EP_STAT estat = _gdp_gob_init_vrfy_ctx(gob);
		if (gob->vrfy_ctx == NULL)
		{
			char ebuf[100];
			ep_dbg_cprintf(Dbg, 3, "_gdp_datum_hash(init): %s\n",
					ep_stat_tostr(estat, ebuf, sizeof ebuf));
			return NULL;
		}
	}

	EP_CRYPTO_MD *md = ep_crypto_md_clone(gob->vrfy_ctx);
	_gdp_datum_digest(datum, md);
	uint8_t mdbuf[EP_CRYPTO_MAX_DIGEST];
	size_t mdlen = sizeof mdbuf;
	ep_crypto_md_final(md, &mdbuf, &mdlen);
	ep_crypto_md_free(md);
	gdp_hash_t *hash = gdp_hash_new(gob->hashalg, mdbuf, mdlen);
	return hash;
}


/*
**  Check a datum hash for equality
*/

bool
gdp_datum_hash_equal(gdp_datum_t *datum,
					const gdp_gin_t *gin,
					const gdp_hash_t *hash)
{
	return _gdp_datum_hash_equal(datum, gin->gob, hash);
}

bool
_gdp_datum_hash_equal(gdp_datum_t *datum,
					gdp_gob_t *gob,
					const gdp_hash_t *hash)
{
	bool r = false;
	gdp_hash_t *newhash = _gdp_datum_hash(datum, gob);

	if (gdp_hash_equal(hash, newhash))
		r = true;
	gdp_hash_free(newhash);
	return r;
}


/*
**  Convert internal datum structure to protobuf-encoded datum.
**  Assumes the protobuf version is empty on entry.
*/

void
_gdp_datum_to_pb(const gdp_datum_t *datum,
				GdpMessage *msg,
				GdpDatum *pbd)
{
	// recno
	pbd->recno = datum->recno;

	// timestamp
	if (EP_TIME_IS_VALID(&datum->ts))
	{
		if (pbd->ts == NULL)
		{
			pbd->ts = (GdpTimestamp *) ep_mem_zalloc(sizeof *pbd->ts);
			gdp_timestamp__init(pbd->ts);
		}
		pbd->ts->sec = datum->ts.tv_sec;
		pbd->ts->has_sec = true;
		pbd->ts->nsec = datum->ts.tv_nsec;
		pbd->ts->has_nsec = pbd->ts->nsec != 0;
		pbd->ts->accuracy = datum->ts.tv_accuracy;
		pbd->ts->has_accuracy = pbd->ts->accuracy != 0.0;
	}
	else if (pbd->ts != NULL)
	{
		ep_dbg_cprintf(Dbg, 3, "_gdp_datum_to_pb: freeing ts\n");
		gdp_timestamp__free_unpacked(pbd->ts, NULL);
		pbd->ts = NULL;
	}

	// data payload
	if (datum->dbuf != NULL && gdp_buf_getlength(datum->dbuf) > 0)
	{
		size_t l = gdp_buf_getlength(datum->dbuf);
		pbd->data.len = l;
		pbd->data.data = (uint8_t *) ep_mem_malloc(l);
		memcpy(pbd->data.data, gdp_buf_getptr(datum->dbuf, l), l);
	}

	// hash of previous record
	if (datum->prevhash == NULL)
		pbd->has_prevhash = false;
	else
	{
		size_t l = gdp_hash_getlength(datum->prevhash);
		pbd->has_prevhash = true;
		pbd->prevhash.len = l;
		if (pbd->prevhash.data != NULL)
			ep_mem_free(pbd->prevhash.data);
		pbd->prevhash.data = (uint8_t *) ep_mem_malloc(l);
		memcpy(pbd->prevhash.data, gdp_hash_getptr(datum->prevhash, NULL), l);
	}

	// signature
	if (datum->sig != NULL)
	{
		if (pbd->sig == NULL)
		{
			pbd->sig = (GdpSignature *) ep_mem_malloc(sizeof *pbd->sig);
			gdp_signature__init(pbd->sig);
		}
		size_t l;
		void *sigdata = gdp_sig_getptr(datum->sig, &l);
		pbd->sig->sig.len = l;
		if (pbd->sig->sig.data != NULL)
			ep_mem_free(pbd->sig->sig.data);
		pbd->sig->sig.data = ep_mem_malloc(l);
		memcpy(pbd->sig->sig.data, sigdata, l);
	}
	else if (pbd->sig != NULL)
	{
		ep_dbg_cprintf(Dbg, 3, "_gdp_datum_to_pb: freeing sig\n");
		gdp_signature__free_unpacked(pbd->sig, NULL);
		pbd->sig = NULL;
	}
}


/*
**  Convert protobuf-encoded datum to internal datum structure.
*/

void
_gdp_timestamp_from_pb(EP_TIME_SPEC *ts,
				const GdpTimestamp *pbd)
{
	if (pbd != NULL)
	{
		ts->tv_sec = pbd->sec;
		ts->tv_nsec = pbd->nsec;
		ts->tv_accuracy = pbd->accuracy;
	}
	else
	{
		EP_TIME_INVALIDATE(ts);
	}
}

void
_gdp_datum_from_pb(gdp_datum_t *datum,
				const GdpDatum *pbd,
				const GdpSignature *sig)
{
	// recno
	datum->recno = pbd->recno;

	// timestamp
	_gdp_timestamp_from_pb(&datum->ts, pbd->ts);

	// data
	if (datum->dbuf == NULL)
		datum->dbuf = gdp_buf_new();
	gdp_buf_write(datum->dbuf, pbd->data.data, pbd->data.len);

	// signature
	if (sig != NULL)
	{
		if (datum->sig == NULL)
			datum->sig = gdp_sig_new(0, NULL, 0);
		gdp_sig_set(datum->sig, sig->sig.data, sig->sig.len);
	}
}


/*
**  Sign a datum
**
**		gob->digest must already be set up to contain the header part
**		of the signature before this is called.  Since this also
**		includes the secret key, it isn't passed in here.
**		The signature is left in the datum.
*/

EP_STAT
_gdp_datum_sign(gdp_datum_t *datum, gdp_gob_t *gob)
{
	EP_STAT estat;

	if (gob->sign_ctx == NULL)
	{
		ep_dbg_cprintf(Dbg, 1, "_gdp_datum_sign: null GOB signature digest\n");
		return GDP_STAT_CRYPTO_SKEY_REQUIRED;
	}

	EP_CRYPTO_MD *md = ep_crypto_md_clone(gob->sign_ctx);
	_gdp_datum_digest(datum, md);

	// now get the final signature
	uint8_t sigbuf[EP_CRYPTO_MAX_SIG];
	size_t siglen = sizeof sigbuf;
	estat = ep_crypto_sign_final(md, sigbuf, &siglen);

	// transfer the signature into the datum
	if (EP_STAT_ISOK(estat))
	{
		if (datum->sig == NULL)
			datum->sig = gdp_sig_new(gob->hashalg, NULL, 0);
		gdp_sig_set(datum->sig, sigbuf, siglen);
	}
	else if (ep_dbg_test(Dbg, 1))
	{
		char ebuf[100];
		ep_dbg_printf("_gdp_datum_sign: ep_crypto_sign_final => %s\n",
					ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}

	// zero out sigbuf (good hygiene)
	memset(sigbuf, 0, sizeof sigbuf);
	return estat;
}


/*
**  Verify datum signature
**
**		This also initializes the message digest (hash) field, even if
**		we don't have a public key, so it's important that this be
**		called even if we don't care about the signature.
*/

EP_STAT
gdp_datum_vrfy(gdp_datum_t *datum, gdp_gin_t *gin)
{
	// if signature already verified, don't do it again
	if (EP_UT_BITSET(GDP_DF_GOODSIG, datum->flags))
	{
		ep_dbg_cprintf(Dbg, 32, "gdp_datum_vrfy: already OK\n");
		return EP_STAT_OK;
	}

	return _gdp_datum_vrfy_gob(datum, gin->gob);
}

EP_STAT
_gdp_datum_vrfy_gob(gdp_datum_t *datum, gdp_gob_t *gob)
{
	EP_STAT estat = EP_STAT_OK;

	if (gob->vrfy_ctx == NULL)
	{
		// sets up message digest hash even if no public key
		estat = _gdp_gob_init_vrfy_ctx(gob);
		if (!EP_STAT_ISOK(estat))
		{
			char ebuf[100];
			ep_dbg_cprintf(Dbg, 30,
					"gdp_datum_vrfy(init): %s\n",
					ep_stat_tostr(estat, ebuf, sizeof ebuf));
			estat = GDP_STAT_CRYPTO_NO_PUB_KEY;
			goto nopubkey;
		}
	}

	// if we aren't verifying we can quit now
	if (!EP_UT_BITSET(GOBF_VERIFYING, gob->flags))
	{
		// not verifying: no public key
		ep_dbg_cprintf(Dbg, 21, "gdp_datum_vrfy: no pub key\n");
		return GDP_STAT_CRYPTO_NO_PUB_KEY;
	}

	// if there's no signature we can give up now
	size_t siglen;
	if (datum->sig == NULL || (siglen = gdp_sig_getlength(datum->sig)) <= 0)
	{
		ep_dbg_cprintf(Dbg, 31, "gdp_datum_vrfy: no signature\n");
		return GDP_STAT_CRYPTO_SIG_MISSING;
	}

	EP_CRYPTO_MD *md = ep_crypto_md_clone(gob->vrfy_ctx);

	_gdp_datum_digest(datum, md);
	size_t len = gdp_sig_getlength(datum->sig);
	estat = ep_crypto_vrfy_final(md, gdp_sig_getptr(datum->sig, NULL), len);
	ep_crypto_md_free(md);
	if (!EP_STAT_ISOK(estat))
	{
		// error: signature failure
		char ebuf[100];
		ep_dbg_cprintf(Dbg, 31, "gdp_datum_vrfy(final): %s\n",
					ep_stat_tostr(estat, ebuf, sizeof ebuf));
		if (EP_UT_BITSET(GOBF_VRFY_WARN, gob->flags))
		{
			ep_dbg_cprintf(Dbg, 31,
						"gdp_datum_vrfy: signature failure (warn)\n");
			estat = EP_STAT_OK;		//XXX: perhaps a warning status?

			// pretend the verification succeeded
			datum->flags |= GDP_DF_GOODSIG;
		}
		else
		{
			ep_dbg_cprintf(Dbg, 1,
						"gdp_datum_vrfy: signature failure (fail)\n");
			estat = GDP_STAT_CRYPTO_VRFY_FAIL;
		}
	}
	else
	{
		// successful verification
		datum->flags |= GDP_DF_GOODSIG;

nopubkey:
		{
			int lev = EP_STAT_ISOK(estat) ? 51 : 31;
			char ebuf[100];
			ep_dbg_cprintf(Dbg, lev, "gdp_datum_vrfy: %s\n",
						ep_stat_tostr(estat, ebuf, sizeof ebuf));
		}
	}
	return estat;
}
