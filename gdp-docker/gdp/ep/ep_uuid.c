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

#include "ep.h"
#include "ep_uuid.h"

#include <errno.h>
#include <string.h>

#if EP_OSCF_UUID_TYPE == EP_UUID_OSSP

static EP_STAT
uuid_stat_to_ep_stat(uuid_rc_t uustat)
{
	switch (uustat)
	{
	  case UUID_RC_OK:
		return EP_STAT_OK;
	  case UUID_RC_ARG:		// invalid argument
		return EP_STAT_INVALID_ARG;
	  case UUID_RC_MEM:		// out of memory
		return EP_STAT_OUT_OF_MEMORY;
	  case UUID_RC_SYS:		// system error
		return ep_stat_from_errno(errno);
	  case UUID_RC_IMP:		// not implemented
		return EP_STAT_NOT_IMPLEMENTED;
	  case UUID_RC_INT:		// internal error
	  default:
		return EP_STAT_SOFTWARE_ERROR;
	}
}

#elif EP_OSCF_UUID_TYPE == EP_UUID_BSD
typedef uint32_t	uuid_rc_t;

static EP_STAT
uuid_stat_to_ep_stat(uuid_rc_t uustat)
{
	switch (uustat)
	{
	  case uuid_s_ok:
		return EP_STAT_OK;
	  case uuid_s_bad_version:
		return EP_STAT_UUID_VERSION;
	  case uuid_s_invalid_string_uuid:
		return EP_STAT_UUID_PARSE_ERROR;
	  case uuid_s_no_memory:
		return EP_STAT_OUT_OF_MEMORY;
	  default:
		return EP_STAT_SOFTWARE_ERROR;
	}
}

#endif

EP_STAT
ep_uuid_generate(EP_UUID *uu)
{
#if EP_OSCF_UUID_TYPE == EP_UUID_OSSP
	uuid_rc_t status = uuid_create(&uu->uu);
	if (status == UUID_RC_OK)
		status = uuid_make(uu->uu, UUID_MAKE_V4);
	return uuid_stat_to_ep_stat(status);
#elif EP_OSCF_UUID_TYPE == EP_UUID_BSD
	uuid_rc_t status;
	uuid_create(&uu->uu, &status);
	return uuid_stat_to_ep_stat(status);
#elif EP_OSCF_UUID_TYPE == EP_UUID_TSO
	uuid_generate(uu->uu);
	return EP_STAT_OK;
# else
	ep_crypto_random_buf(uu->uu, sizeof uu->uu);
	// make this a version 4 (random-based) UUID
	uu->uu[6] = (uu->uu[6] & 0x0f) | 0x40;
	return EP_STAT_OK;
#endif
}

EP_STAT
ep_uuid_destroy(EP_UUID *uu)
{
#if EP_OSCF_UUID_TYPE == EP_UUID_OSSP
	uuid_rc_t status = uuid_destroy(uu->uu);
	uu->uu = NULL;
	return uuid_stat_to_ep_stat(status);
#else
	return EP_STAT_OK;
#endif
}

EP_STAT
ep_uuid_tostr(EP_UUID *uu, EP_UUID_STR str)
{
#if EP_OSCF_UUID_TYPE == EP_UUID_OSSP
	uuid_rc_t status;
	size_t strsize = sizeof str;
	status = uuid_export(uu->uu, UUID_FMT_STR, str, &strsize);
	return uuid_stat_to_ep_stat(status);
#elif EP_OSCF_UUID_TYPE == EP_UUID_BSD
	uuid_rc_t status;
	char *s = NULL;

	uuid_to_string(&uu->uu, &s, &status);
	if (s != NULL)
	{
		memcpy(str, s, sizeof(EP_UUID_STR));
		free(s);
	}
	return uuid_stat_to_ep_stat(status);
#elif EP_OSCF_UUID_TYPE == EP_UUID_TSO
	uuid_unparse(uu->uu, str);
	return EP_STAT_OK;
#else
	snprintf(str, EP_UUID_STR_LEN,
			"%02x%02x%02x%02x-"
			"%02x%02x-"
			"%02x%02x-"
			"%02x%02x-"
			"%02x%02x%02x%02x%02x%02x",
			uu->uu[0],  uu->uu[1],  uu->uu[2],  uu->uu[3],
			uu->uu[4],  uu->uu[5],  uu->uu[6],  uu->uu[7],
			uu->uu[8],  uu->uu[9],  uu->uu[10], uu->uu[12],
			uu->uu[12], uu->uu[13], uu->uu[14], uu->uu[15]);
	return EP_STAT_OK;
#endif
}


EP_STAT
ep_uuid_parse(EP_UUID *uu, EP_UUID_STR str)
{
#if EP_OSCF_UUID_TYPE == EP_UUID_OSSP
	uuid_rc_t status = uuid_import(uu->uu, UUID_FMT_STR, str, sizeof str);
	return uuid_stat_to_ep_stat(status);
#elif EP_OSCF_UUID_TYPE == EP_UUID_BSD
	uuid_rc_t status;
	uuid_from_string(str, &uu->uu, &status);
	return uuid_stat_to_ep_stat(status);
#elif EP_OSCF_UUID_TYPE == EP_UUID_TSO
	int status = uuid_parse(str, uu->uu);
	if (status == 0)
		return EP_STAT_OK;
	return EP_STAT_UUID_PARSE_ERROR;
#else
	int i = sscanf(str,
			"%02hhx%02hhx%02hhx%02hhx-"
			"%02hhx%02hhx-"
			"%02hhx%02hhx-"
			"%02hhx%02hhx-"
			"%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
			&uu->uu[0],  &uu->uu[1],  &uu->uu[2],  &uu->uu[3],
			&uu->uu[4],  &uu->uu[5],  &uu->uu[6],  &uu->uu[7],
			&uu->uu[8],  &uu->uu[9],  &uu->uu[10], &uu->uu[12],
			&uu->uu[12], &uu->uu[13], &uu->uu[14], &uu->uu[15]);
	if (i != 16)
		return EP_STAT_UUID_PARSE_ERROR;
	return EP_STAT_OK;
#endif
}

EP_STAT
ep_uuid_clear(EP_UUID *uu)
{
#if EP_OSCF_UUID_TYPE == EP_UUID_OSSP
	uuid_rc_t status = uuid_load(uu->uu, "nil");
	return uuid_stat_to_ep_stat(status);
#elif EP_OSCF_UUID_TYPE == EP_UUID_BSD
	uuid_rc_t status;
	uuid_create_nil(&uu->uu, &status);
	return uuid_stat_to_ep_stat(status);
#elif EP_OSCF_UUID_TYPE == EP_UUID_TSO
	uuid_clear(uu->uu);
	return EP_STAT_OK;
#else
	memset(uu->uu, 0, sizeof uu->uu);
	return EP_STAT_OK;
#endif
}

bool
ep_uuid_is_null(EP_UUID *uu)
{
#if EP_OSCF_UUID_TYPE == EP_UUID_OSSP
	uuid_rc_t status;		// does this ever get set?
	int result;
	status = uuid_isnil(uu->uu, &result);
	return result != 0;
#elif EP_OSCF_UUID_TYPE == EP_UUID_BSD
	uuid_rc_t status;		// does this ever get set?
	return uuid_is_nil(&uu->uu, &status) != 0;
#elif EP_OSCF_UUID_TYPE == EP_UUID_TSO
	return uuid_is_null(uu->uu) != 0;
#else
	static uint8_t zero[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	return memcmp(uu->uu, zero, sizeof zero) == 0;
#endif
}

int
ep_uuid_compare(EP_UUID *uu1, EP_UUID *uu2)
{
#if EP_OSCF_UUID_TYPE == EP_UUID_OSSP
	int result;
	uuid_rc_t status = uuid_compare(uu1->uu, uu2->uu, &result);
	return result;
#elif EP_OSCF_UUID_TYPE == EP_UUID_BSD
	uuid_rc_t status;		// no way to return status, if any
	return uuid_compare(&uu1->uu, &uu2->uu, &status);
#elif EP_OSCF_UUID_TYPE == EP_UUID_TSO
	return uuid_compare(uu1->uu, uu2->uu);
#else
	return memcmp(uu1->uu, uu2->uu, sizeof uu1->uu);
#endif
}
