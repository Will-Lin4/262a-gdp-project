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

#include <ep/ep.h>
#include <ep/ep_dbg.h>
#include <ep/ep_string.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/types.h>

#include "gdp_zc_client.h"

#if GDP_OSCF_USE_ZEROCONF

static EP_DBG	Dbg = EP_DBG_INIT("gdp.zc.client", "GDP Zeroconf client");
static EP_DBG	DemoMode = EP_DBG_INIT("_demo", "Demo Mode");

typedef enum
{
	COMMAND_HELP,
	COMMAND_VERSION,
	COMMAND_BROWSE_SERVICES,
	COMMAND_BROWSE_ALL_SERVICES,
	COMMAND_BROWSE_DOMAINS,
#if defined(HAVE_GDBM) || defined(HAVE_DBM)
	COMMAND_DUMP_STDB,
#endif
} command;

typedef struct config
{
	int verbose;
	int terminate_on_all_for_now;
	int terminate_on_cache_exhausted;
	const char *domain;
	const char *stype;
	int ignore_local;
	command comm;
	int resolve;
	int no_fail;
	int parsable;
#if defined(HAVE_GDBM) || defined(HAVE_DBM)
	int no_db_lookup;
#endif
} config_t;

typedef struct service_info service_info_t;

struct service_info
{
	AvahiIfIndex interface;
	AvahiProtocol protocol;
	char *name, *type, *domain;
	AvahiServiceResolver *resolver;
	config_t *conf;
	AVAHI_LLIST_FIELDS(service_info_t, info);
};

static AvahiSimplePoll *SimplePoll = NULL;
static AvahiClient *Client = NULL;
static int NAllForNow = 0, NCacheExhausted = 0, NResolving = 0;
static AvahiStringList *BrowsedTypes = NULL;
static service_info_t *Services = NULL;
static int NColumns = 80;
static int Browsing = 0;
static zcinfo_t **InfoList = NULL;
#define MAX_PORT_LEN 5
#define MAX_ADDR_LEN 50

static int
infolist_append_front(zcinfo_t **list, zcinfo_t *info)
{
	info->info_next = *list;
	*list = info;
	return 0;
}

static char *
make_printable(const char *from, char *to)
{
	const char *f;
	char *t;

	for (f = from, t = to; *f; f++, t++)
		*t = isprint(*f) ? *f : '_';

	*t = 0;

	return to;
}


/*
**	Print something about zeroconf.  It appears to be only for debugging.
*/

static void
print_service_line(config_t *conf,
		char c,
		AvahiIfIndex interface,
		AvahiProtocol protocol,
		const char *name,
		const char *type,
		const char *domain,
		int nl)
{
	char ifname[IF_NAMESIZE];

#if defined(HAVE_GDBM) || defined(HAVE_DBM)
	if (!conf->no_db_lookup)
		type = stdb_lookup(type);
#endif

	if (conf->parsable)
	{
		char sn[AVAHI_DOMAIN_NAME_MAX], *e = sn;
		size_t l = sizeof(sn);

		printf("%c;%s;%s;%s;%s;%s%s",
			c,
			interface != AVAHI_IF_UNSPEC ? if_indextoname(interface, ifname) : "n/a",
			protocol != AVAHI_PROTO_UNSPEC ? avahi_proto_to_string(protocol) : "n/a",
			avahi_escape_label(name, strlen(name), &e, &l), type, domain, nl ? "\n" : "");
	}
	else
	{
		char label[AVAHI_LABEL_MAX];
		make_printable(name, label);

		printf("%c %6s %4s %-*s %-20s %s\n",
			c,
			interface != AVAHI_IF_UNSPEC ? if_indextoname(interface, ifname) : "n/a",
			protocol != AVAHI_PROTO_UNSPEC ? avahi_proto_to_string(protocol) : "n/a",
			NColumns - 35, label, type, domain);
	}
	fflush(stdout);
}

static void
remove_service(config_t *c, service_info_t *i)
{
	assert(c);
	assert(i);

	AVAHI_LLIST_REMOVE(service_info_t, info, Services, i);

	if (i->resolver)
		avahi_service_resolver_free(i->resolver);

	avahi_free(i->name);
	avahi_free(i->type);
	avahi_free(i->domain);
	avahi_free(i);
}

static void
check_terminate(config_t *c)
{
	assert(NAllForNow >= 0);
	assert(NCacheExhausted >= 0);
	assert(NResolving >= 0);

	if (NAllForNow <= 0 && NResolving <= 0)
	{

		if (c->verbose && !c->parsable)
		{
			printf(": All for now\n");
			NAllForNow++; /* Make sure that this event is not repeated */
		}

		if (c->terminate_on_all_for_now)
		{
			avahi_simple_poll_quit(SimplePoll);
		}
	}

	if (NCacheExhausted <= 0 && NResolving <= 0)
	{

		if (c->verbose && !c->parsable)
		{
			printf(": Cache exhausted\n");
			NCacheExhausted++; /* Make sure that this event is not repeated */
		}

		if (c->terminate_on_cache_exhausted)
		{
			avahi_simple_poll_quit(SimplePoll);
		}
	}
}

static void
service_resolver_callback(AvahiServiceResolver *r,
		AvahiIfIndex interface,
		AvahiProtocol protocol,
		AvahiResolverEvent event,
		const char *name,
		const char *type,
		const char *domain,
		const char *host_name,
		const AvahiAddress *a,
		uint16_t port,
		AvahiStringList *txt,
		AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
		void *userdata)
{
	service_info_t *i = (service_info_t *) userdata;

	assert(r);
	assert(i);

	switch (event)
	{
		case AVAHI_RESOLVER_FOUND:
		{
			char *t;
			char address[AVAHI_ADDRESS_STR_MAX];
			struct in_addr unused_address;
			zcinfo_t *info = (zcinfo_t *) avahi_malloc(sizeof(zcinfo_t));

			avahi_address_snprint(address, sizeof(address), a);

			t = avahi_string_list_to_string(txt);

			if (i->conf->verbose)
			{
				print_service_line(i->conf, '=', interface, protocol,
						name, type, domain, 0);

				if (i->conf->parsable)
					printf(";%s;%s;%u;%s\n",
						host_name,
						address,
						port,
						t);
				else
					printf("   hostname = [%s]\n"
						"   address = [%s]\n"
						"   port = [%u]\n"
						"   txt = [%s]\n",
						host_name,
						address,
						port,
						t);
			}

			/* Assume that if it's not ipv6, then it's ipv4 */
			if (inet_pton(AF_INET6, address, &unused_address))
			{
				char format_addr[AVAHI_ADDRESS_STR_MAX+2];
				snprintf(format_addr, sizeof(format_addr), "[%s]", address);
				info->address = avahi_strdup(format_addr);
			}
			else
			{
				info->address = avahi_strdup(address);
			}

			info->xinfo = t;

			/* Append zcinfo here */
			info->port = port;
			infolist_append_front(InfoList, info);
			break;
		}

		case AVAHI_RESOLVER_FAILURE:
			fprintf(stderr,
					"Failed to resolve service '%s' of type '%s' in domain '%s': %s\n",
					name, type, domain,
					avahi_strerror(avahi_client_errno(Client)));
			break;
	}


	avahi_service_resolver_free(i->resolver);
	i->resolver = NULL;

	assert(NResolving > 0);
	NResolving--;
	check_terminate(i->conf);
	fflush(stdout);
}

static service_info_t *
add_service(config_t *c,
		AvahiIfIndex interface,
		AvahiProtocol protocol,
		const char *name,
		const char *type,
		const char *domain)
{
	service_info_t *i;

	i = avahi_new(service_info_t, 1);

	if (c->resolve)
	{
		if (!(i->resolver = avahi_service_resolver_new(Client, interface,
						protocol, name, type, domain, AVAHI_PROTO_UNSPEC,
						(AvahiLookupFlags) 0, service_resolver_callback, i)))
		{
			avahi_free(i);
			fprintf(stderr,
					"Failed to resolve service '%s' of type '%s' in domain '%s': %s\n",
					name, type, domain,
					avahi_strerror(avahi_client_errno(Client)));
			return NULL;
		}

		NResolving++;
	}
	else
	{
		i->resolver = NULL;
	}

	i->interface = interface;
	i->protocol = protocol;
	i->name = avahi_strdup(name);
	i->type = avahi_strdup(type);
	i->domain = avahi_strdup(domain);
	i->conf = c;

	AVAHI_LLIST_PREPEND(service_info_t, info, Services, i);

	return i;
}

static service_info_t *
find_service(AvahiIfIndex interface,
		AvahiProtocol protocol,
		const char *name,
		const char *type,
		const char *domain)
{
	service_info_t *i;

	for (i = Services; i; i = i->info_next)
		if (i->interface == interface &&
				i->protocol == protocol &&
				strcasecmp(i->name, name) == 0 &&
				avahi_domain_equal(i->type, type) &&
				avahi_domain_equal(i->domain, domain))
			return i;
	return NULL;
}

static void
service_browser_callback(AvahiServiceBrowser *b,
		AvahiIfIndex interface,
		AvahiProtocol protocol,
		AvahiBrowserEvent event,
		const char *name,
		const char *type,
		const char *domain,
		AvahiLookupResultFlags flags,
		void *userdata)
{

	config_t *c = (config_t *) userdata;

	assert(b);
	assert(c);

	ep_dbg_cprintf(Dbg, 20, "service_browser_callback: event %d\n", event);

	switch (event)
	{
		case AVAHI_BROWSER_NEW:
		{
			ep_dbg_cprintf(Dbg, 20, "service_browser_callback: AVAHI_BROWSER_NEW\n");
			if (c->ignore_local && (flags & AVAHI_LOOKUP_RESULT_LOCAL))
				break;

			if (find_service(interface, protocol, name, type, domain))
				return;

			add_service(c, interface, protocol, name, type, domain);

			if (c->verbose)
				print_service_line(c, '+', interface, protocol, name,
						type, domain, 1);
			break;

		}

		case AVAHI_BROWSER_REMOVE:
		{
			service_info_t *info;

			ep_dbg_cprintf(Dbg, 20, "service_browser_callback: AVAHI_BROWSER_REMOVE\n");
			if (!(info = find_service(interface, protocol, name, type, domain)))
				return;

			remove_service(c, info);

			if (c->verbose)
				print_service_line(c, '-', interface, protocol, name,
						type, domain, 1);
			break;
		}

		case AVAHI_BROWSER_FAILURE:
			ep_dbg_cprintf(Dbg, 20, "service_browser_callback: AVAHI_BROWSER_REMOVE: %s\n",
					avahi_strerror(avahi_client_errno(Client)));
			fprintf(stderr,
					"service_browser failed: %s\n",
					avahi_strerror(avahi_client_errno(Client)));
			avahi_simple_poll_quit(SimplePoll);
			break;

		case AVAHI_BROWSER_CACHE_EXHAUSTED:
			ep_dbg_cprintf(Dbg, 20, "service_browser_callback: AVAHI_BROWSER_CACHE_EXHAUSTED\n");
			NCacheExhausted --;
			check_terminate(c);
			break;

		case AVAHI_BROWSER_ALL_FOR_NOW:
			ep_dbg_cprintf(Dbg, 20, "service_browser_callback: AVAHI_BROWSER_ALL_FOR_NOW\n");
			NAllForNow --;
			check_terminate(c);
			break;
	}
}

static void
browse_service_type(config_t *c,
		const char *stype,
		const char *domain)
{
	AvahiServiceBrowser *b;
	AvahiStringList *i;

	assert(c);
	assert(Client);
	assert(stype);

	ep_dbg_cprintf(Dbg, 34, "browse_service_type: stype \"%s\" domain \"%s\"\n",
			stype, domain);

	for (i = BrowsedTypes; i; i = i->next)
	{
		ep_dbg_cprintf(Dbg, 34, "  ... \"%s\"\n", i->text);
		if (avahi_domain_equal(stype, (char*) i->text))
			return;
	}

	ep_dbg_cprintf(Dbg, 34, "browse_service_type: avahi_service_browser_new\n");
	if (!(b = avahi_service_browser_new(
			Client,
			AVAHI_IF_UNSPEC,
			AVAHI_PROTO_UNSPEC,
			stype,
			domain,
			(AvahiLookupFlags) 0,
			service_browser_callback,
			c)))
	{
		fprintf(stderr,
				"avahi_service_browser_new() failed: %s\n",
				avahi_strerror(avahi_client_errno(Client)));
		avahi_simple_poll_quit(SimplePoll);
	}

	BrowsedTypes = avahi_string_list_add(BrowsedTypes, stype);

	NAllForNow++;
	NCacheExhausted++;
}

static int
start(config_t *conf)
{

	assert(!Browsing);

	const char *version, *hn;

	//XXX why is this here?  version is never used
	ep_dbg_cprintf(Dbg, 34, "start: avahi_client_get_version_string\n");
	if (!(version = avahi_client_get_version_string(Client)))
	{
		fprintf(stderr,
				"Failed to query version string: %s\n",
				avahi_strerror(avahi_client_errno(Client)));
		return -1;
	}

	//XXX why is this here?  hn is never used
	ep_dbg_cprintf(Dbg, 34, "start: avahi_client_get_host_name_fqdn\n");
	if (!(hn = avahi_client_get_host_name_fqdn(Client)))
	{
		fprintf(stderr,
				"Failed to query host name: %s\n",
				avahi_strerror(avahi_client_errno(Client)));
		return -1;
	}

	browse_service_type(conf, conf->stype, conf->domain);

	Browsing = 1;
	ep_dbg_cprintf(Dbg, 34, "start: returning 0\n");
	return 0;
}

static int create_new_simple_poll_client(config_t *conf);

static void
client_callback(AvahiClient *c,
		AvahiClientState state,
		AVAHI_GCC_UNUSED void * userdata)
{
	config_t *conf = (config_t *) userdata;

	/*
		This function might be called when avahi_client_new() has not
		returned yet.
	*/
	Client = c;

	ep_dbg_cprintf(Dbg, 20, "client_callback: state %d\n", state);
	switch (state)
	{
		case AVAHI_CLIENT_FAILURE:
			if (conf->no_fail && avahi_client_errno(c) == AVAHI_ERR_DISCONNECTED)
			{
				/* We have been disconnected, so let reconnect */
				fprintf(stderr, "Disconnected, reconnecting ...\n");

				avahi_client_free(Client);
				Client = NULL;

				avahi_string_list_free(BrowsedTypes);
				BrowsedTypes = NULL;

				while (Services)
					remove_service(conf, Services);

				Browsing = 0;

				(void) create_new_simple_poll_client(conf);
			}
			else
			{
				fprintf(stderr,
						"Client failure, exiting: %s\n",
						avahi_strerror(avahi_client_errno(c)));
				avahi_simple_poll_quit(SimplePoll);
			}
			break;

		case AVAHI_CLIENT_S_REGISTERING:
		case AVAHI_CLIENT_S_RUNNING:
		case AVAHI_CLIENT_S_COLLISION:
			if (!Browsing)
				if (start(conf) < 0)
				{
					avahi_simple_poll_quit(SimplePoll);
				}
			break;

		case AVAHI_CLIENT_CONNECTING:
			if (conf->verbose && !conf->parsable)
				fprintf(stderr, "Waiting for daemon ...\n");
			break;
	}
}

static int
init_config(config_t *c)
{
	assert(c);

	c->terminate_on_cache_exhausted = 0;
	c->terminate_on_all_for_now = 0;
	c->ignore_local = 0;
	c->no_fail = 0;
	c->parsable = 0;
	c->verbose = ep_dbg_test(DemoMode, 1);
	c->resolve = 1;
	c->terminate_on_all_for_now = 1;

	c->stype = ep_adm_getstrparam("swarm.gdp.zeroconf.proto", "_gdp._tcp");
	c->domain = ep_adm_getstrparam("swarm.gdp.zeroconf.domain", "local");

	return 0;
}

static int
create_new_simple_poll_client(config_t *conf)
{
	int error = 0;

	Client = avahi_client_new(avahi_simple_poll_get(SimplePoll),
			conf->no_fail ? AVAHI_CLIENT_NO_FAIL : (AvahiClientFlags) 0,
			client_callback,
			conf,
			&error);
	if (Client == NULL)
	{
		ep_dbg_cprintf(Dbg, 1,
				"create_new_simple_poll_client: avahi_simple_poll_get: %s\n",
				avahi_strerror(error));
		if (conf->verbose)
			fprintf(stderr, "Failed to create client object: %s\n",
					avahi_strerror(error));
		avahi_simple_poll_quit(SimplePoll);
	}
	ep_dbg_cprintf(Dbg, 34, "create_new_simple_poll_client => %d\n", error);
	return error;
}

/////////////////////// Public API below ///////////////////////


/**
 *
 */
int
gdp_zc_scan()
{
	int error;
	config_t conf;
	int rval = 0;

	init_config(&conf);

	// clean up from possible previous call
	Browsing = false;
	if (SimplePoll != NULL)
		avahi_simple_poll_free(SimplePoll);
	if (BrowsedTypes != NULL)
		avahi_string_list_free(BrowsedTypes);
	BrowsedTypes = NULL;
	NAllForNow = NResolving = NCacheExhausted = 0;


	if ((SimplePoll = avahi_simple_poll_new()) == NULL)
	{
		ep_dbg_cprintf(Dbg, 1, "gdp_zc_scan: avahi_simple_poll_new failed: %s\n",
				strerror(errno));
		goto fail0;
	}

	if ((error = create_new_simple_poll_client(&conf)) != 0)
	{
		ep_dbg_cprintf(Dbg, 1, "gdp_zc_scan: create_new_simple_poll_client failed: %s\n",
				strerror(errno));
		goto fail1;
	}

	InfoList = (zcinfo_t**) avahi_malloc(sizeof(zcinfo_t*));
	*InfoList = NULL;
	ep_dbg_cprintf(Dbg, 30, "gdp_zc_scan: starting avahi_simple_poll_loop\n");
	avahi_simple_poll_loop(SimplePoll);
	rval = 1;

fail1:
	ep_dbg_cprintf(Dbg, 34, "gdp_zc_scan: avahi_simple_poll_free\n");
	avahi_simple_poll_free(SimplePoll);
	SimplePoll = NULL;
fail0:
	// stub
	return rval;
}

/**
 * reversing isn't on purpose, just the easiest way to copy
 */
static zcinfo_t **
list_copy_reverse(zcinfo_t **list)
{
	zcinfo_t *i, *k, **newlist;

	newlist = (zcinfo_t**) avahi_malloc(sizeof(zcinfo_t*));
	*newlist = NULL;
	for (i = *list, k = *newlist; i; i = i->info_next)
	{
		k = (zcinfo_t *) avahi_malloc(sizeof(zcinfo_t));
		k->port = i->port;
		k->address = ep_mem_strdup(i->address);
		infolist_append_front(newlist, k);
	}
	return newlist;
}

int
list_length(zcinfo_t **list)
{
	zcinfo_t *i;
	int count;

	for (i = *list, count = 0; i; i = i->info_next)
		count++;

	return count;
}

/**
 * zero indexed linked list pop
 * format is "addr:port;"
 * notice the colon and semicolon
 */
static char *
list_pop_str(zcinfo_t **list, int index)
{
	zcinfo_t *info, *tmpinfo;
	char *outstr;
	int length, total_strlen, i;

	length = list_length(list);
	total_strlen = length * ((MAX_PORT_LEN+2) + MAX_ADDR_LEN) + 1;
	outstr = (char *) avahi_malloc(sizeof(char) * total_strlen);
	*outstr = '\0';
	info = *list;
	if (length == 0)
	{
		avahi_free(outstr);
		return NULL;
	}
	else
	{
		if (index == 0)
		{ // Removing the "head" node
			snprintf(outstr, total_strlen, "%s:%u;", info->address, info->port);
			if (length != 1)
			{
				*list = info->info_next;
			}
			// free old head
			avahi_free(info->address);
			avahi_free(info);
		}
		else
		{
			// reach the node right before the node to remove
			for (i = 0; i < index-1; i++)
			{
				info = info->info_next;
			}
			snprintf(outstr, total_strlen, "%s:%u;",
					info->info_next->address,
					info->info_next->port);
			// remove next node from list
			tmpinfo = info;
			info = info->info_next;
			tmpinfo->info_next = tmpinfo->info_next->info_next;
			// free node
			avahi_free(info->address);
			avahi_free(info);
		}
	}
	return outstr;
}

char *
gdp_zc_addr_str(zcinfo_t **list)
{
	zcinfo_t **listcopy;
	char *outstr, *tmpstr;
	int length, total_strlen, randnum;

	listcopy = list_copy_reverse(list);
	length = list_length(listcopy);
	total_strlen = length * (MAX_PORT_LEN + MAX_ADDR_LEN) + 1;
	outstr = (char *) avahi_malloc(sizeof(char) * total_strlen);
	*outstr = '\0';
	srand(time(NULL));
	while(length > 0)
	{
		randnum = rand() % length;
		tmpstr = list_pop_str(listcopy, randnum);
		strlcat(outstr, tmpstr, total_strlen);
		avahi_free(tmpstr);
		length--;
	}
	avahi_free(listcopy);
	// if len > 0 then remove last char which will be ';'
	if (strlen(outstr) > 0)
	{
		outstr[strlen(outstr) - 1] = '\0';
	}
	return outstr;
}

zcinfo_t **
gdp_zc_get_infolist()
{
	return InfoList;
}

int
gdp_zc_free_infolist(zcinfo_t **list)
{
	zcinfo_t *i, *next;
	for (i = *list; i;)
	{
		avahi_free(i->address);
		avahi_free(i->xinfo);
		next = i->info_next;
		avahi_free(i);
		i = next;
	}
	avahi_free(list);
	list = NULL;
	return 1;
}

#endif // GDP_OSCF_USE_ZEROCONF

/* vim: set noexpandtab : */
