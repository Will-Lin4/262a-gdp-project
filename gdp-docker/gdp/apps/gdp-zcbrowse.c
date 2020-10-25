/*
**	----- BEGIN LICENSE BLOCK -----
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
**	----- END LICENSE BLOCK -----
*/

#include <gdp/gdp_zc_client.h>


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
#if GDP_OSCF_USE_ZEROCONF
	zcinfo_t *i, **list;
	char *zcstr;

	printf("start browse\n");
	if (gdp_zc_scan())
	{
		printf("getting info\n");
		/* always need to retrieve list */
		list = gdp_zc_get_infolist();

		/* you can access info as a linked list */
		for (i = *list; i; i = i->info_next)
		{
			printf("host:%s port: %d\n", i->address, i->port);
		}

		/* or you can access info as a string */
		zcstr = gdp_zc_addr_str(list);
		if (zcstr)
		{
			printf("list: %s\n", zcstr);
			/* need to free the string after you're done */
			free(zcstr);
		}
		else
		{
			printf("list fail\n");
		}

		/* you always need to free the list after you're done */
		printf("freeing info\n");
		gdp_zc_free_infolist(list);
		return 0;
	}
	else
	{
		return 1;
	}
#else // GDP_OSCF_USE_ZEROCONF
	fprintf(stderr, "gdp_zcbrowse: no Avahi available\n");
	return 1;
#endif // GDP_OSCF_USE_ZEROCONF
}

/* vim: set noexpandtab : */
