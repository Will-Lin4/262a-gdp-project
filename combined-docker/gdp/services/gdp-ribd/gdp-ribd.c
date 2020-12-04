/*
**  The gdp-ribd service (leveraging MariaDB and OQGraph) handles
**  requests to add or withdraw network adjacencies from a GDP
**  network, and also handles queries to find (anycast) or mfind
**  (multicast) the best next hop or hops (i.e. shortest weighted
**  path) to a specific destination from the requestor's location in
**  the network.
**
**  This daemon (previously named gdp-directoryd) began life as mere
**  temporary development scaffolding to facilitate the transition to
**  GDP net4 PDUs and a newly implemented GDP v2 forwarding plane, but
**  like anything which works a little too well while requiring far
**  too little attention, it has been extended beyond original intent.
**
**	----- BEGIN LICENSE BLOCK -----
**	Applications for the Global Data Plane
**	From the Ubiquitous Swarm Lab, 490 Cory Hall, U.C. Berkeley.
**
**	Copyright (c) 2017-2019, Regents of the University of California.
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
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <mysql.h>
#include <errno.h>
#include <assert.h>
#include "gdp/gdp.h"
#include "gdp-ribd.h"

// must match database password in gdp-rib.sql
#define LOOPBACK_IP "127.0.0.1"
#define USER "gdp_rib_user"
#define IDENTIFIED_BY "gdp_rib_pass"

#define GDP_NAME_HEX_FORMAT (2 * sizeof(gdp_name_t))
#define GDP_NAME_HEX_STRING (GDP_NAME_HEX_FORMAT + 1)
char dguid_s[GDP_NAME_HEX_STRING];
char eguid_s[GDP_NAME_HEX_STRING];

// prevent mysql_con server-side closure when gdp-ribd is not being used at all
#define MYSQL_CON_KEEPALIVE_SECS 570
char query_expire[] = "call gdp_rib.drop_expired();";

char call_add_nhop_pre[] = "call gdp_rib.add_nhop (x'";
char call_add_nhop_sep[] = "', x'";
char call_add_nhop_end[] = "');";

char call_delete_nhop_pre[] = "call gdp_rib.delete_nhop (x'";
char call_delete_nhop_sep[] = "', x'";
char call_delete_nhop_end[] = "');";

char call_find_nhop_pre[] = "call gdp_rib.find_nhop (x'";
char call_find_nhop_sep[] = "', x'";
char call_find_nhop_end[] = "');";

char call_mfind_nhop_pre[] = "call gdp_rib.mfind_nhop (x'";
char call_mfind_nhop_sep[] = "', x'";
char call_mfind_nhop_end[] = "');";

char call_flush_nhop_pre[] = "call gdp_rib.flush_nhops (x'";
char call_flush_nhop_end[] = "');";

// FIXME approximate size plus margin of error
#define GDP_QUERY_STRING (3 * (2 * sizeof(gdp_name_t)) + 256)
char query[GDP_QUERY_STRING];

otw_dir_t otw_dir;

int main(int argc, char **argv)
{
	struct timeval tv = { .tv_sec = MYSQL_CON_KEEPALIVE_SECS, .tv_usec = 0 };
	struct sockaddr_in si_loc;
	struct sockaddr_in si_rem;
	socklen_t si_rem_len = sizeof(si_rem);
	int fd_listen;
	int on = 1;
	int otw_dir_len;
	MYSQL *mysql_con;
	MYSQL_RES *mysql_result;
	unsigned int mysql_fields;
	MYSQL_ROW mysql_row;
	unsigned int rows;
	gdp_pname_t _tmp_pname_1;
	gdp_pname_t _tmp_pname_2;					

	// enforce assumption in request handling loop (optimization)
	assert(GDP_CMD_DIR_FOUND == GDP_CMD_DIR_FIND + 1);
	assert(GDP_CMD_DIR_MFOUND == GDP_CMD_DIR_MFIND + 1);

	if ((fd_listen = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		perror("socket");
		exit(1);
	}

	// blocking sockets are sufficient for this simple/temp directory service
	// fcntl(fd_listen, F_SETFL, O_NONBLOCK);
	fcntl(fd_listen, F_SETFD, FD_CLOEXEC);
	assert(0 <=
		   setsockopt(fd_listen, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)));
	// periodically remove inactive guids
	assert(0 <=
		   setsockopt(fd_listen, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)));
	
	si_loc.sin_family = AF_INET;
	si_loc.sin_port = htons(PORT);
	si_loc.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(fd_listen, (struct sockaddr *)&si_loc, sizeof(si_loc)) < 0)
	{
		perror("bind");
		exit(1);
	}

 new_session:
		
	mysql_con = mysql_init(NULL);
	if (mysql_con == NULL)
	{
		perror("mysql_init");
		exit(1);
	}

	// wait for mariadb availability
	while (mysql_real_connect(mysql_con, LOOPBACK_IP, USER, IDENTIFIED_BY,
							  NULL, 0, NULL, 0) == NULL)
	{
		perror(mysql_error(mysql_con));
		sleep(2);
	}
	printf("gdp-ribd mysql connection established\n");
	
 new_rx:
		
	// await request (blocking)
	otw_dir_len = recvfrom(fd_listen, (uint8_t *) &otw_dir.ver, sizeof(otw_dir),
						   MSG_TRUNC, (struct sockaddr *)&si_rem, &si_rem_len);
	// timeout used to expire rows
	if (otw_dir_len < 0 && errno == EAGAIN)
	{
		debug(INFO, "\nprocess expired rows ");
		if (mysql_query(mysql_con, query_expire))
		{
			fprintf(stderr, "Error: %s (%s)\n", mysql_error(mysql_con),
					query_expire);
			mysql_close(mysql_con);
			goto new_session;
		}
		debug(INFO, "= %llu\n", mysql_affected_rows(mysql_con));

		// ensure "commands out of sync" error does not occur on next query
		mysql_result = mysql_store_result(mysql_con);
		goto mysql_result_cleanup;
	}

	// continue if too short/long or partial field
	if (otw_dir_len < offsetof(otw_dir_t, eguid[0]) ||
		otw_dir_len > sizeof(otw_dir_t) ||
		((otw_dir_len - offsetof(otw_dir_t, eguid[0])) % sizeof(gdp_name_t) !=
		 0))
	{
		debug(INFO, "invalid length %d ver 0x%x cmd %d id 0x%x\n",
			  otw_dir_len, otw_dir.ver, otw_dir.cmd, otw_dir.id);
		goto new_rx;
	}

	if (otw_dir.ver != GDP_CHAN_PROTO_VERSION)
	{
		debug(WARN, "Warn: unrecognized packet from %s:%d len %d",
			  inet_ntoa(si_rem.sin_addr),
			  ntohs(si_rem.sin_port), otw_dir_len);
		goto new_rx;
	}

	debug(INFO, "\nReceived packet len %d from %s:%d\n",
		  otw_dir_len, inet_ntoa(si_rem.sin_addr), ntohs(si_rem.sin_port));

	// handle request
	switch (otw_dir.cmd)
	{

	case GDP_CMD_DIR_ADD:
	{
		char *q;

		debug(INFO, "id(0x%x) cmd -> add nhop\n"
			  "\teguid[%s]\n"
			  "\tdguid[%s]\n",
			  ntohs(otw_dir.id),
			  gdp_printable_name(otw_dir.eguid[0], _tmp_pname_1),
			  gdp_printable_name(otw_dir.dguid, _tmp_pname_2));

		// build the query

		strcpy(query, call_add_nhop_pre);
		q = query + sizeof(call_add_nhop_pre) - 1;

		debug(INFO, "-> eguid[");
		for (int i = 0; i < sizeof(gdp_name_t); i++)
		{
			debug(INFO, "%.2x", otw_dir.eguid[0][i]);
			sprintf(q + (i * 2), "%.2x", otw_dir.eguid[0][i]);
		}
		debug(INFO, "]\n");
		q += (2 * sizeof(gdp_name_t));

		strcat(q, call_add_nhop_sep);
		q += sizeof(call_add_nhop_sep) - 1;

		debug(INFO, "-> dguid[");
		for (int i = 0; i < sizeof(gdp_name_t); i++)
		{
			debug(INFO, "%.2x", otw_dir.dguid[i]);
			sprintf(q + (i * 2), "%.2x", otw_dir.dguid[i]);
		}
		debug(INFO, "]\n");
		q += (2 * sizeof(gdp_name_t));

		strcat(q, call_add_nhop_end);
		q += sizeof(call_add_nhop_end) - 1;
	}
	break;

	case GDP_CMD_DIR_DELETE:
	{
		char *q;

		debug(INFO, "id(0x%x) cmd -> delete nhop\n"
			  "\teguid[%s]\n"
			  "\tdguid[%s]\n",
			  ntohs(otw_dir.id),
			  gdp_printable_name(otw_dir.eguid[0], _tmp_pname_1),
			  gdp_printable_name(otw_dir.dguid, _tmp_pname_2));

		// build the query

		strcpy(query, call_delete_nhop_pre);
		q = query + sizeof(call_delete_nhop_pre) - 1;

		debug(INFO, "-> eguid[");
		for (int i = 0; i < sizeof(gdp_name_t); i++)
		{
			debug(INFO, "%.2x", otw_dir.eguid[0][i]);
			sprintf(q + (i * 2), "%.2x", otw_dir.eguid[0][i]);
		}
		debug(INFO, "]\n");
		q += (2 * sizeof(gdp_name_t));

		strcat(q, call_delete_nhop_sep);
		q += sizeof(call_delete_nhop_sep) - 1;

		debug(INFO, "-> dguid[");
		for (int i = 0; i < sizeof(gdp_name_t); i++)
		{
			debug(INFO, "%.2x", otw_dir.dguid[i]);
			sprintf(q + (i * 2), "%.2x", otw_dir.dguid[i]);
		}
		debug(INFO, "]\n");
		q += (2 * sizeof(gdp_name_t));

		strcat(q, call_delete_nhop_end);
		q += sizeof(call_delete_nhop_end) - 1;
	}
	break;

	case GDP_CMD_DIR_FIND:
	{
		char *q;
			
		debug(INFO, "id(0x%x) cmd -> find nhop\n"
			  "\teguid[%s]\n"
			  "\tdguid[%s]\n",
			  ntohs(otw_dir.id),
			  gdp_printable_name(otw_dir.eguid[0], _tmp_pname_1),
			  gdp_printable_name(otw_dir.dguid, _tmp_pname_2));

		// build the query

		strcpy(query, call_find_nhop_pre);
		q = query + sizeof(call_find_nhop_pre) - 1;

		debug(INFO, "-> eguid[");
		for (int i = 0; i < sizeof(gdp_name_t); i++)
		{
			debug(INFO, "%.2x", otw_dir.eguid[0][i]);
			sprintf(q + (i * 2), "%.2x", otw_dir.eguid[0][i]);
		}
		debug(INFO, "]\n");
		q += (2 * sizeof(gdp_name_t));
			
		strcat(q, call_find_nhop_sep);
		q += sizeof(call_find_nhop_sep) - 1;

		debug(INFO, "-> dguid[");
		for (int i = 0; i < sizeof(gdp_name_t); i++)
		{
			debug(INFO, "%.2x", otw_dir.dguid[i]);
			sprintf(q + (i * 2), "%.2x", otw_dir.dguid[i]);
		}
		debug(INFO, "]\n");
		q += (2 * sizeof(gdp_name_t));
			
		strcat(q, call_find_nhop_end);
		q += sizeof(call_find_nhop_end) - 1;
	}
	break;

	case GDP_CMD_DIR_MFIND:
	{
		char *q;
			
		debug(INFO, "id(0x%x) cmd -> mfind nhop\n"
			  "\teguid[%s]\n"
			  "\tdguid[%s]\n",
			  ntohs(otw_dir.id),
			  gdp_printable_name(otw_dir.eguid[0], _tmp_pname_1),
			  gdp_printable_name(otw_dir.dguid, _tmp_pname_2));

		// build the query

		strcpy(query, call_mfind_nhop_pre);
		q = query + sizeof(call_mfind_nhop_pre) - 1;

		debug(INFO, "-> eguid[");
		for (int i = 0; i < sizeof(gdp_name_t); i++)
		{
			debug(INFO, "%.2x", otw_dir.eguid[0][i]);
			sprintf(q + (i * 2), "%.2x", otw_dir.eguid[0][i]);
		}
		debug(INFO, "]\n");
		q += (2 * sizeof(gdp_name_t));
			
		strcat(q, call_mfind_nhop_sep);
		q += sizeof(call_mfind_nhop_sep) - 1;

		debug(INFO, "-> dguid[");
		for (int i = 0; i < sizeof(gdp_name_t); i++)
		{
			debug(INFO, "%.2x", otw_dir.dguid[i]);
			sprintf(q + (i * 2), "%.2x", otw_dir.dguid[i]);
		}
		debug(INFO, "]\n");
		q += (2 * sizeof(gdp_name_t));
			
		strcat(q, call_mfind_nhop_end);
		q += sizeof(call_mfind_nhop_end) - 1;
	}
	break;

	case GDP_CMD_DIR_FLUSH:
	{
		char *q;

		debug(INFO, "id(0x%x) cmd -> flush nhops\n"
			  "\teguid[%s]\n",
			  ntohs(otw_dir.id),
			  gdp_printable_name(otw_dir.eguid[0], _tmp_pname_1));

		// build the query

		strcpy(query, call_flush_nhop_pre);
		q = query + sizeof(call_flush_nhop_pre) - 1;

		debug(INFO, "-> eguid[");
		for (int i = 0; i < sizeof(gdp_name_t); i++)
		{
			debug(INFO, "%.2x", otw_dir.eguid[0][i]);
			sprintf(q + (i * 2), "%.2x", otw_dir.eguid[0][i]);
		}
		debug(INFO, "]\n");
		q += (2 * sizeof(gdp_name_t));

		strcat(q, call_flush_nhop_end);
		q += sizeof(call_flush_nhop_end) - 1;
	}
	break;

	default:
	{
		fprintf(stderr, "Error: packet with unknown cmd\n");
		goto new_rx;
	}
	break;

	} // switch

	// query the database and get the result
	if (mysql_query(mysql_con, query))
	{
		fprintf(stderr, "Error: %s (%s)\n", mysql_error(mysql_con), query);
		mysql_close(mysql_con);
		goto new_session;
	}
	mysql_result = mysql_store_result(mysql_con);

	// post-processing
	switch (otw_dir.cmd)
	{

	case GDP_CMD_DIR_FLUSH:
	{
		debug(INFO, "ackless");
		goto mysql_result_cleanup;
	}
	break;
			
	case GDP_CMD_DIR_ADD:
	case GDP_CMD_DIR_DELETE:
	{
		debug(INFO, "ack");
		otw_dir_len = sendto(fd_listen, (uint8_t *) &otw_dir.ver,
							 otw_dir_len, 0, (struct sockaddr *)&si_rem,
							 sizeof(si_rem));
	}
	break;

	case GDP_CMD_DIR_FIND:
	case GDP_CMD_DIR_MFIND:
	{
		rows = 0;
		if (mysql_result == NULL)
		{
			fprintf(stderr, "Error: %s (%s)\n", mysql_error(mysql_con),
					query);
			// no mysql_results to cleanup
			mysql_close(mysql_con);
			goto new_session;
		}

		mysql_fields = mysql_field_count(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);
		if (!mysql_row || mysql_fields != 1 || mysql_row[0] == NULL)
		{
			debug(INFO, "no nhop\n");
			++rows;
			memset(&otw_dir.eguid[rows - 1][0], 0, sizeof(gdp_name_t));
		}
		else
		{
			// prepare reply (FIND to FOUND or MFIND to MFOUND)
			otw_dir.cmd += 1;

			while (mysql_row && rows < GDP_CMD_DIR_EGUID_MAX) {
				++rows;
				memcpy(&otw_dir.eguid[rows - 1][0],
					   (uint8_t *) mysql_row[0], sizeof(gdp_name_t));
					
				debug(INFO, "\teguid[%s]\n",
					  gdp_printable_name(otw_dir.eguid[rows - 1],
										 _tmp_pname_1));
					
				debug(INFO, "<- eguid[");
				for (int i = 0; i < sizeof(gdp_name_t); i++)
				{
					debug(INFO, "%.2x",
						  (uint8_t) otw_dir.eguid[rows - 1][i]);
				}
				debug(INFO, "]\n");
						
				mysql_row = mysql_fetch_row(mysql_result);
			}
			if (otw_dir.cmd == GDP_CMD_DIR_FOUND && rows != 1)
			{
				debug(ERR, "find row count invalid %d", rows);
				otw_dir_len = -1;
				goto mysql_result_cleanup;
			}
			if (otw_dir.cmd == GDP_CMD_DIR_MFOUND && rows == 0)
			{
				debug(ERR, "mfind row count invalid %d", rows);
				otw_dir_len = -1;
				goto mysql_result_cleanup;
			}
		}

		otw_dir_len = offsetof(otw_dir_t, eguid[0]) + (rows *
													   sizeof(gdp_name_t));
		otw_dir_len = sendto(fd_listen, (uint8_t *) &otw_dir.ver,
							 otw_dir_len, 0, (struct sockaddr *)&si_rem,
							 sizeof(si_rem));
	}
	break;

	default:
	{
		fprintf(stderr, "Error: unsupported cmd\n");
		goto mysql_result_cleanup;
	}
	break;
	
	} // switch
	
	// sendto diagnostics
	if (otw_dir_len < 0)
	{
		debug(ERR, "Error: id(0x%x) send len %d error %s\n",
			  ntohs(otw_dir.id), otw_dir_len, strerror(errno));
	}
	else
	{
		debug(INFO, "id(0x%x) send len %d\n",
			  ntohs(otw_dir.id), otw_dir_len);
	}

 mysql_result_cleanup:
		
	// ensure "commands out of sync" error does not occur on next query
	if (mysql_result != NULL)
	{
		debug(INFO, "mysql results found and freed\n");
		mysql_free_result(mysql_result);
		while (mysql_more_results(mysql_con))
		{
			debug(INFO, "more results found and freed\n");
			if (mysql_next_result(mysql_con) == 0)
			{
				mysql_result = mysql_store_result(mysql_con);
				mysql_free_result(mysql_result);
			}
		}
	}

	// carry on
	goto new_rx;
}
