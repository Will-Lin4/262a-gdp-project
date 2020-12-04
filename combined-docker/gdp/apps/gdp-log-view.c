/* vim: set ai sw=4 sts=4 ts=4 : */

/***********************************************************************
**  ----- BEGIN LICENSE BLOCK -----
**	Applications for the Global Data Plane
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
***********************************************************************/

#include <ep/ep.h>
#include <ep/ep_dbg.h>
#include <ep/ep_hash.h>
#include <ep/ep_hexdump.h>
#include <ep/ep_net.h>
#include <ep/ep_prflags.h>
#include <ep/ep_string.h>
#include <ep/ep_time.h>
#include <ep/ep_xlate.h>
#include <gdp/gdp.h>

#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <sys/stat.h>

// leverage existing code (this is a hack!)
#define GDP_LOG_VIEW	1
#define Dbg				DbgLogdSqlite
#include "../gdplogd/logd_sqlite.c"
#undef Dbg
#include "../gdp/gdp_hongd.h"


/*
**  GDP-LOG-VIEW --- display raw on-disk storage
**
**		Not for user consumption.
**		This does peek into private header files.
*/

static EP_DBG	Dbg = EP_DBG_INIT("gdp-log-view", "Dump GDP logs for debugging");


#define LIST_NO_METADATA		0x00000001	// only list logs with no metadata


void
pr_pubkey(uint32_t md_id, uint32_t md_len, uint8_t *mdata, int plev)
{
	{
		int keylen = mdata[2] << 8 | mdata[3];
		printf("\tmd_alg %s (%d), keytype %s (%d), keylen %d\n",
				ep_crypto_md_alg_name(mdata[0]), mdata[0],
				ep_crypto_keytype_name(mdata[1]), mdata[1],
				keylen);
	}
	if (plev > 1)
	{
		EP_CRYPTO_KEY *key;

		key = ep_crypto_key_read_mem(mdata + 4,
						md_len - 4,
						EP_CRYPTO_KEYFORM_DER,
						EP_CRYPTO_F_PUBLIC);
		ep_crypto_key_print(key, stdout, EP_CRYPTO_F_PUBLIC);
		ep_crypto_key_free(key);
	}
	if (plev >= 4)
		ep_hexdump(mdata + 4, md_len - 4, stdout, EP_HEXDUMP_HEX, 0);
}


EP_STAT
pr_verbose_metadata(gdp_gob_t *gob, int plev)
{
	gdp_md_t *md = gob->gob_md;

	printf("\n    --------------- Metadata ---------------\n");
	if (md == NULL)
	{
		fprintf(stderr, "%s: no metadata\n", gob->pname);
		return EP_STAT_ERROR;		//XXX something more specific?
	}

	int mdx = 0;
	for (;; ++mdx)
	{
		gdp_md_id_t md_id;
		size_t md_len;
		const uint8_t *md_data;
		EP_STAT estat;

		estat = gdp_md_get(md, mdx, &md_id, &md_len, (const void **) &md_data);
		EP_STAT_CHECK(estat, break);

		uint8_t mdata[md_len + 1];			// +1 for null-terminator
		memcpy(mdata, md_data, md_len);
		mdata[md_len] = '\0';

		if (plev > 1)
		{
			int printtype = 0;
			fprintf(stdout,
					"\nMetadata entry %d: name 0x%08" PRIx32
					", len %zd",
					mdx, md_id, md_len);
			switch (md_id)
			{
				case GDP_MD_XID:
					printf(" (external id)\n");
					printtype = 1;
					break;

				case GDP_MD_NONCE:
					printf(" (nonce)\n    ");
					ep_hexdump(mdata, md_len, stdout, EP_HEXDUMP_TERSE, 0);
					printtype = 0;
					break;

				case GDP_MD_CTIME:
					printf(" (creation time)\n");
					printtype = 1;
					break;

				case GDP_MD_EXPIRE:
					printf(" (expiration time)\n");
					printtype = 1;
					break;

				case GDP_MD_CREATOR:
					printf(" (creator)\n");
					printtype = 1;
					break;

				case GDP_MD_PUBKEY:
					printf(" (public key, deprecated)\n");
					pr_pubkey(md_id, md_len, mdata, plev);
					break;

				case GDP_MD_OWNERPUBKEY:
					printf(" (owner public key)\n");
					pr_pubkey(md_id, md_len, mdata, plev);
					break;

				case GDP_MD_WRITERPUBKEY:
					printf(" (writer public key)\n");
					pr_pubkey(md_id, md_len, mdata, plev);
					break;

				case GDP_MD_SYNTAX:
					printf(" (syntax)\n");
					printtype = 1;
					break;

				case GDP_MD_LOCATION:
					printf(" (location)\n");
					printtype = 1;
					break;

				default:
					printf("\n");
					printtype = 1;
					break;
			}
			if (printtype == 1)
			{
				printf("\t%s", EpChar->lquote);
				ep_xlate_out(mdata, md_len,
						stdout, "", EP_XLATE_PLUS | EP_XLATE_NPRINT);
				fprintf(stdout, "%s\n", EpChar->rquote);
			}
			else if (printtype == 2)
			{
				ep_hexdump(mdata, md_len, stdout, EP_HEXDUMP_HEX, 0);
			}
			else if (printtype == 3)
			{
				ep_hexdump(mdata, md_len, stdout, EP_HEXDUMP_ASCII, 0);
			}
		}
		else if (md_id == GDP_MD_XID)
		{
			fprintf(stdout, "\tExternal name: %s\n", mdata);
		}
		else if (md_id == GDP_MD_CREATOR)
		{
			fprintf(stdout, "\tCreator:       %s\n", mdata);
		}
		else if (md_id == GDP_MD_CTIME)
		{
			fprintf(stdout, "\tCreation Time: %s\n", mdata);
		}

		if (plev >= 4)
		{
			ep_hexdump(mdata, md_len, stdout, EP_HEXDUMP_ASCII, 0);
		}
	}
	return EP_STAT_OK;
}


static EP_STAT
show_rec(EP_STAT estat, gdp_datum_t *datum, gdp_result_ctx_t *ctx)
{
	uintptr_t ptrint = (uintptr_t) ctx;
	uint32_t pflags = (uint32_t) ptrint;
	if (EP_STAT_IS_SAME(estat, GDP_STAT_ACK_CONTENT))
		gdp_datum_print(datum, stdout, pflags);
	else
		ep_app_message(estat, "show_rec: unknown estat");
	return EP_STAT_OK;
}


EP_STAT
show_log(gdp_name_t log_name, int plev)
{
	gdp_pname_t log_pname;
	gdp_gob_t *gob = NULL;
	EP_STAT estat;

	(void) gdp_printable_name(log_name, log_pname);
	if (plev <= 0)
	{
		printf("%s\n", log_pname);
		return EP_STAT_OK;
	}

	// create a GOB data structure for this log
	estat = _gdp_gob_new(log_name, &gob);
	EP_STAT_CHECK(estat, return estat);
	gob->x = (struct gdp_gob_xtra *) ep_mem_zalloc(sizeof *gob->x);
	_gdp_gob_lock(gob);

	// open the on-disk database
	estat = sqlite_open(gob);
	EP_STAT_CHECK(estat, goto fail0);

	// figure out what we want to actually print
	if (plev == 1)
	{
		// show log name, number of records, external name if known
		size_t md_len;
		const void *md_data;

		estat = gdp_md_find(gob->gob_md, GDP_MD_XID, &md_len, &md_data);
		if (!EP_STAT_ISOK(estat))
		{
			md_data = (void *) "-";
			md_len = 1;
		}
		printf("%s %" PRIgdp_recno " %.*s\n",
				log_pname, gob->nrecs, (int) md_len, (const char *) md_data);
		goto done;
	}
	if (plev == 2)
	{
		// show log name, nrecs, brief metadata overview
		printf("%s %" PRIgdp_recno "\n", log_pname, gob->nrecs);
		gdp_md_t *md = gob->gob_md;
		if (md == NULL)
			goto done;
		int mdx = 0;
		for (;; mdx++)
		{
			gdp_md_id_t md_id;
			size_t md_len;
			const uint8_t *md_data;

			estat = gdp_md_get(md, mdx, &md_id, &md_len, (const void **) &md_data);
			EP_STAT_CHECK(estat, break);

			int ptype = 0;
			const char *ptag = NULL;
			switch (md_id)
			{
				case GDP_MD_XID:
					ptag = "external id";
					ptype = 1;
					break;
				case GDP_MD_NONCE:
					ptag = "nonce";
					ptype = 2;
					break;
				case GDP_MD_CTIME:
					ptag = "creation time";
					ptype = 1;
					break;
				case GDP_MD_EXPIRE:
					ptag = "expiration time";
					ptype = 1;
					break;
				case GDP_MD_CREATOR:
					ptag = "creator";
					ptype = 1;
					break;
				case GDP_MD_PUBKEY:
					ptag = "public key, deprecated";
					break;
				case GDP_MD_OWNERPUBKEY:
					ptag = "owner public key";
					break;
				case GDP_MD_WRITERPUBKEY:
					ptag = "writer public key";
					break;
				default:
					printf("       0x%08x len %zd ", md_id, md_len);
					break;
			}
			printf("  %22s ", ptag == NULL ? "" : ptag);
			switch (ptype)
			{
			case 1:
				// print as string
				printf("%s", EpChar->lquote);
				ep_xlate_out(md_data, md_len, stdout, "",
						EP_XLATE_PLUS | EP_XLATE_NPRINT);
				printf("%s\n", EpChar->rquote);
				break;

			case 2:
				// print as hex
				ep_hexdump(md_data, md_len, stdout, EP_HEXDUMP_TERSE, 0);
				break;

			default:
				printf("(omitted)\n");
				break;
			}
		}
		goto done;
	}

	if (plev >= 3)
		printf("\n\n----------------------------------------------------------\n");
	printf("%s: %" PRIgdp_recno " recs\n", log_pname, gob->nrecs);
	uint32_t pflags = GDP_DATUM_PRMETAONLY;
	if (plev >= 3)
	{
		pflags |= GDP_DATUM_PRSIG;
		pr_verbose_metadata(gob, plev);
	}
	if (plev >= 5)
		pflags &= ~GDP_DATUM_PRMETAONLY;

	// now dump the contents
	if (plev >= 4)
	{
		printf("\n    --------------- Data ---------------\n");
		uintptr_t intptrpflags = pflags;
		estat = sqlite_read_by_recno(gob, 1, UINT32_MAX, show_rec,
								(void *) intptrpflags);
	}

	if (false)
	{
		char ebuf[60];
fail0:
		fprintf(stderr, "%s: %s\n",
				log_pname, ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
done:
	if (gob != NULL)
		_gdp_gob_free(&gob);
	return estat;
}


int
list_logs(const char *gcl_dir_name, int plev)
{
	DIR *dir;
	int subdir;
	gdp_name_t gcl_iname;

	ep_dbg_cprintf(Dbg, 11, "list_logs(%s)\n", gcl_dir_name);
	dir = opendir(gcl_dir_name);
	if (dir == NULL)
	{
		fprintf(stderr, "Could not open %s, errno = %d\n",
				gcl_dir_name, errno);
		return EX_NOINPUT;
	}
	closedir(dir);

	for (subdir = 0; subdir < 0x100; subdir++)
	{
		char dbuf[400];

		snprintf(dbuf, sizeof dbuf, "%s/_%02x", gcl_dir_name, subdir);
		dir = opendir(dbuf);
		if (dir == NULL)
			continue;

		for (;;)
		{
			struct dirent *dent;

			// read the next directory entry
			dent = readdir(dir);
			if (dent == NULL)
				break;

			// we're only interested in .data files
			char *p = strrchr(dent->d_name, '.');
			if (p == NULL || strcmp(p, GLOG_SUFFIX) != 0)
				continue;

			// save the full pathname in case we need it
			snprintf(dbuf, sizeof dbuf, "%s/_%02x/%s",
					gcl_dir_name, subdir, dent->d_name);

			// strip off the ".db"
			*p = '\0';

			// print the name and maybe other info
			gdp_parse_name(dent->d_name, gcl_iname);
			show_log(gcl_iname, plev);
		}
		closedir(dir);
	}

	return EX_OK;
}


void
usage(const char *msg)
{
	fprintf(stderr,
			"Usage error: %s\n"
			"Usage: log-view [-d dir] [-D dbgspec ] [-l] [-r] [-v] [log_name ...]\n"
			"\t-d dir -- set log database root directory\n"
			"\t-D spec -- set debug flags\n"
			"\t-l -- list all local logs\n"
			"\t-n -- only list logs with no metadata\n"
			"\t-v -- print verbose information (-vv for more detail)\n",
				msg);

	exit(EX_USAGE);
}

int
main(int argc, char *argv[])
{
	int opt;
	int verbosity = 0;
	bool ls_logs = false;
	char *log_xname = NULL;
	const char *log_dir_name = NULL;
	EP_STAT estat;

//	ep_lib_init(0);

	while ((opt = getopt(argc, argv, "d:D:lnv")) > 0)
	{
		switch (opt)
		{
		case 'd':
			log_dir_name = optarg;
			break;

		case 'D':
			ep_dbg_set(optarg);
			break;

		case 'l':
			ls_logs = true;
			break;

		case 'v':
			verbosity++;
			break;

		default:
			usage("unknown flag");
		}
	}
	argc -= optind;
	argv += optind;

	// initialization
	estat = gdp_init_phase_0(NULL, 0);
	EP_STAT_CHECK(estat, goto fail0);
	ep_adm_readparams("gdplogd");
	_gdp_name_init(NULL);

	// initialize logd_sqlite
	estat = sqlite_init(log_dir_name);
	EP_STAT_CHECK(estat, goto fail0);

	if (ls_logs)
	{
		if (argc > 0)
			usage("cannot use a log name with -l");
		return list_logs(LogDir, verbosity);
	}

	if (argc <= 0)
		usage("log name required");
	for (; argc > 0; argc--, argv++)
	{
		gdp_name_t log_name;
		log_xname = argv[0];

		estat = gdp_parse_name(log_xname, log_name);
		if (!EP_STAT_ISOK(estat))
		{
			ep_app_message(estat, "unparsable log name");
			continue;
		}
		estat = show_log(log_name, verbosity);
	}
fail0:
	if (EP_STAT_ISFAIL(estat))
		exit(EX_UNAVAILABLE);
	exit(EX_OK);
}
