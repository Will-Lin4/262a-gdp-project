/* vim: set ai sw=4 sts=4 ts=4 :*/

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

#include "gdp_zc_server.h"

#if GDP_OSCF_USE_ZEROCONF

#include <ep/ep_assert.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>


static AvahiEntryGroup *AGroup = NULL;
static AvahiSimplePoll *SimplePoll = NULL;
static char *SName = NULL;
static uint16_t SPort = 0;

static void create_services(AvahiClient *c);

static void
entry_group_callback(AvahiEntryGroup *g,
		AvahiEntryGroupState state,
		AVAHI_GCC_UNUSED void *userdata)
{
	assert(g == AGroup || AGroup == NULL);
	AGroup = g;

	/* Called whenever the entry group state changes */

	switch (state)
	{
		case AVAHI_ENTRY_GROUP_ESTABLISHED:
			/* The entry group has been established successfully */
			fprintf(stderr,
					"Service '%s' successfully established.\n",
					SName);
			break;

		case AVAHI_ENTRY_GROUP_COLLISION:
		{
			char *n;

			/* A service name collision with a remote service
			 * happened. Let's pick a new name */
			n = avahi_alternative_service_name(SName);
			avahi_free(SName);
			SName = n;

			fprintf(stderr,
					"Service name collision, renaming service to '%s'\n",
					SName);

			/* And recreate the services */
			create_services(avahi_entry_group_get_client(g));
			break;
		}

		case AVAHI_ENTRY_GROUP_FAILURE:
			fprintf(stderr,
					"Entry group failure: %s\n",
					avahi_strerror
					(
					 avahi_client_errno( avahi_entry_group_get_client(g))
					));

			/* Some kind of failure happened while we were registering our services */
			avahi_simple_poll_quit(SimplePoll);
			break;

		case AVAHI_ENTRY_GROUP_UNCOMMITED:
		case AVAHI_ENTRY_GROUP_REGISTERING:
			break;

		default:
			EP_ASSERT_FAILURE("Unknown state %d", state);
	}
}

static void
create_services(AvahiClient *c)
{
	char *n, r[128];
	int ret;
	assert(c);

	/* If this is the first time we're called, let's create a new
	 * entry group if necessary */

	if (!AGroup)
	{
		if (!(AGroup = avahi_entry_group_new(c, entry_group_callback, NULL)))
		{
			fprintf(stderr,
					"avahi_entry_group_new() failed: %s\n",
					avahi_strerror(avahi_client_errno(c)));
			goto fail;
		}
	}

	/* If the group is empty (either because it was just created, or
	 * because it was reset previously, add our entries.  */

	if (avahi_entry_group_is_empty(AGroup))
	{
		fprintf(stderr, "Adding service '%s'\n", SName);

		/* Create some random TXT data */
		snprintf(r, sizeof(r), "random=%i", rand());

		if ((ret = avahi_entry_group_add_service(
						AGroup,					// group
						AVAHI_IF_UNSPEC,		// interface
						AVAHI_PROTO_UNSPEC,		// protocol
						(AvahiPublishFlags) 0,	// flags
						SName,					// service name
						"_gdp._tcp",			// service type
						NULL,					// domain
						NULL,					// host
						SPort,					// port number
						r,						// TXT entry
						NULL)					// end of vararg TXT list
					) < 0)
		{
			if (ret == AVAHI_ERR_COLLISION)
				goto collision;

			fprintf(stderr,
					"Failed to add _gdp._tcp service: %s\n",
					avahi_strerror(ret));
			goto fail;
		}

		/* Tell the server to register the service */
		if ((ret = avahi_entry_group_commit(AGroup)) < 0)
		{
			fprintf(stderr,
					"Failed to commit entry group: %s\n",
					avahi_strerror(ret));
			goto fail;
		}
	}

	return;

collision:

	/* A service name collision with a local service happened. Let's
	 * pick a new name */
	n = avahi_alternative_service_name(SName);
	avahi_free(SName);
	SName = n;

	fprintf(stderr,
			"Service name collision, renaming service to '%s'\n",
			SName);

	avahi_entry_group_reset(AGroup);

	create_services(c);
	return;

fail:
	avahi_simple_poll_quit(SimplePoll);
}

static void
client_callback(AvahiClient *c,
		AvahiClientState state,
		AVAHI_GCC_UNUSED void * userdata)
{
	assert(c);

	/* Called whenever the client or server state changes */

	switch (state)
	{
		case AVAHI_CLIENT_S_RUNNING:
			/* The server has startup successfully and registered its host
			 * name on the network, so it's time to create our services */
			create_services(c);
			break;

		case AVAHI_CLIENT_FAILURE:
			fprintf(stderr,
					"Client failure: %s\n",
					avahi_strerror(avahi_client_errno(c)));
			avahi_simple_poll_quit(SimplePoll);
			break;

		case AVAHI_CLIENT_S_COLLISION:
			/* Let's drop our registered services. When the server is back
			 * in AVAHI_SERVER_RUNNING state we will register them
			 * again with the new host name. */

		case AVAHI_CLIENT_S_REGISTERING:
			/* The server records are now being established. This
			 * might be caused by a host name change. We need to wait
			 * for our own records to register until the host name is
			 * properly esatblished. */
			if (AGroup)
				avahi_entry_group_reset(AGroup);
			break;

		case AVAHI_CLIENT_CONNECTING:
			break;

		default:
			EP_ASSERT_FAILURE("Unknown state %d", state);
	}
}

int
gdp_zc_publish(const char *instance, uint16_t port_no)
{
	AvahiClient *client = NULL;
	int error;
	int ret = 1;

	/* Allocate main loop object */
	if (!(SimplePoll = avahi_simple_poll_new()))
	{
		fprintf(stderr, "Failed to create simple poll object.\n");
		goto fail;
	}

	SName = avahi_strdup(instance);
	SPort = port_no;

	/* Allocate a new client */
	client = avahi_client_new(avahi_simple_poll_get(SimplePoll),
			(AvahiClientFlags) 0, client_callback, NULL, &error);

	/* Check whether creating the client object succeeded */
	if (!client)
	{
		fprintf(stderr, "Failed to create client: %s\n", avahi_strerror(error));
		goto fail;
	}

	/* Run the main loop */
	avahi_simple_poll_loop(SimplePoll);

	ret = 0;

fail:

	/* Cleanup things */

	if (client)
		avahi_client_free(client);

	if (SimplePoll)
		avahi_simple_poll_free(SimplePoll);

	avahi_free(SName);

	return ret;
}

int gdp_zc_shutdown()
{
	return 0;
}

#endif // GDP_OSCF_USE_ZEROCONF

/* vim: set noexpandtab : */
