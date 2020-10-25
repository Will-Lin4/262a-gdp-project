/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**	Advertise (publish) information about the GDP router
**
**		This should really be done automatically by the GDP Router
**		itself, but for the time being this is a stop-gap.
**
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


#define GDP_PORT_DEFAULT			8007

#include <gdp/gdp_zc_server.h>

#include <ep/ep.h>
#include <ep/ep_app.h>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>


void
usage(void)
{
	fprintf(stderr,
			"Usage: %s [-i instance] [-p port]\n",
			ep_app_getprogname());
	exit(EX_USAGE);
}


int
main(int argc, char **argv)
{
#if GDP_OSCF_USE_ZEROCONF
	uint16_t port;
	char *instance = NULL;
	char instancebuf[120];
	int opt;

	ep_lib_init(0);
	ep_adm_readparams("gdp");
	port = ep_adm_getintparam("swarm.gdp.router.port", GDP_PORT_DEFAULT);
	while ((opt = getopt(argc, argv, "i:p:")) > 0)
	{
		switch (opt)
		{
		case 'i':				// instance name
			instance = optarg;
			break;

		case 'p':
			port = atoi(optarg);
			break;

		default:
			usage();
		}
	}

	if (instance == NULL)
	{
		char hostname[64];

		if (gethostname(hostname, sizeof hostname) < 0)
			ep_app_abort("cannot get host name");
		snprintf(instancebuf, sizeof instancebuf, "GDP Router on %s", hostname);
		instance = instancebuf;
	}

	printf("advertise gdp '%s' on %d\n", instance, port);
	gdp_zc_publish(instance, port);
	printf("do other stuff here\n");
	sleep(300);
	printf("terminating...\n");
	return 0;
#else // GDP_OSCF_USE_ZEROCONF
	fprintf(stderr, "gdp-zcpublish: no Avahi available\n");
	return 1;
#endif // GDP_OSCF_USE_ZEROCONF
}

/* vim: set noexpandtab : */
