/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**	GDP_MSG.C --- manipulate protobuf messages
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
#include <ep/ep_log.h>
#include <ep/ep_prflags.h>
#include <ep/ep_stat.h>

#include "gdp.h"
#include "gdp_chan.h"
#include "gdp_priv.h"

#include <event2/event.h>

#include <string.h>
#include <sys/errno.h>

static EP_DBG	Dbg = EP_DBG_INIT("gdp.msg", "GDP message manipulation");

/*
**  Create a new GDP message.
**		The gdp_msg_t type is really a synonym for GdpMessage.
**		In OO terms, this is a Factory.
*/

gdp_msg_t *
_gdp_msg_new(gdp_cmd_t cmd, gdp_rid_t rid, gdp_l5seqno_t l5seqno)
{
	gdp_msg_t *msg;

	ep_dbg_cprintf(Dbg, 24,
				"_gdp_msg_new: cmd %s (%d), rid %" PRIgdp_rid
							" l5seqno %" PRIgdp_l5seqno "\n",
				_gdp_proto_cmd_name(cmd), cmd, rid, l5seqno);

	EP_ASSERT(cmd >= 0 && cmd <= 255);
	msg = (gdp_msg_t *) ep_mem_zalloc(sizeof *msg);
	gdp_message__init(msg);
	msg->cmd = cmd;
	if (rid != GDP_PDU_NO_RID)
	{
		msg->has_rid = true;
		msg->rid = rid;
	}
	if (l5seqno != GDP_PDU_NO_L5SEQNO)
	{
		msg->has_l5seqno = true;
		msg->l5seqno = l5seqno;
	}

	// initialize command body based on command type
	switch (cmd)
	{
	case GDP_CMD_CREATE:
		msg->body_case = GDP_MESSAGE__BODY_CMD_CREATE;
		msg->cmd_create = (GdpMessage__CmdCreate *)
					ep_mem_zalloc(sizeof *msg->cmd_create);
		gdp_message__cmd_create__init(msg->cmd_create);
		msg->cmd_create->metadata = (GdpMetadata *)
					ep_mem_zalloc(sizeof *msg->cmd_create->metadata);
		gdp_metadata__init(msg->cmd_create->metadata);
		break;

	case GDP_CMD_OPEN_AO:
	case GDP_CMD_OPEN_RO:
	case GDP_CMD_OPEN_RA:
		msg->body_case = GDP_MESSAGE__BODY_CMD_OPEN;
		msg->cmd_open = (GdpMessage__CmdOpen *)
					ep_mem_zalloc(sizeof *msg->cmd_open);
		gdp_message__cmd_open__init(msg->cmd_open);
		break;

	case GDP_CMD_APPEND:
		msg->body_case = GDP_MESSAGE__BODY_CMD_APPEND;
		msg->cmd_append = (GdpMessage__CmdAppend *)
					ep_mem_zalloc(sizeof *msg->cmd_append);
		gdp_message__cmd_append__init(msg->cmd_append);
		msg->cmd_append->dl = (GdpDatumList *)
					ep_mem_zalloc(sizeof (GdpDatumList));
		gdp_datum_list__init(msg->cmd_append->dl);
		break;

	case GDP_CMD_READ_BY_RECNO:
		msg->body_case = GDP_MESSAGE__BODY_CMD_READ_BY_RECNO;
		msg->cmd_read_by_recno = (GdpMessage__CmdReadByRecno *)
					ep_mem_zalloc(sizeof *msg->cmd_read_by_recno);
		gdp_message__cmd_read_by_recno__init(msg->cmd_read_by_recno);
		break;

	case GDP_CMD_READ_BY_TS:
		msg->body_case = GDP_MESSAGE__BODY_CMD_READ_BY_TS;
		msg->cmd_read_by_ts = (GdpMessage__CmdReadByTs *)
					ep_mem_zalloc(sizeof *msg->cmd_read_by_ts);
		gdp_message__cmd_read_by_ts__init(msg->cmd_read_by_ts);
		break;

	case GDP_CMD_READ_BY_HASH:
		msg->body_case = GDP_MESSAGE__BODY_CMD_READ_BY_HASH;
		msg->cmd_read_by_hash = (GdpMessage__CmdReadByHash *)
					ep_mem_zalloc(sizeof *msg->cmd_read_by_hash);
		gdp_message__cmd_read_by_hash__init(msg->cmd_read_by_hash);
		break;

	case GDP_CMD_SUBSCRIBE_BY_RECNO:
		msg->body_case = GDP_MESSAGE__BODY_CMD_SUBSCRIBE_BY_RECNO;
		msg->cmd_subscribe_by_recno = (GdpMessage__CmdSubscribeByRecno *)
						ep_mem_zalloc(sizeof *msg->cmd_subscribe_by_recno);
		gdp_message__cmd_subscribe_by_recno__init(
						msg->cmd_subscribe_by_recno);
		break;

	case GDP_CMD_SUBSCRIBE_BY_TS:
		msg->body_case = GDP_MESSAGE__BODY_CMD_SUBSCRIBE_BY_TS;
		msg->cmd_subscribe_by_ts = (GdpMessage__CmdSubscribeByTs *)
						ep_mem_zalloc(sizeof *msg->cmd_subscribe_by_ts);
		gdp_message__cmd_subscribe_by_ts__init(
						msg->cmd_subscribe_by_ts);
		break;

	case GDP_CMD_SUBSCRIBE_BY_HASH:
		msg->body_case = GDP_MESSAGE__BODY_CMD_SUBSCRIBE_BY_HASH;
		msg->cmd_subscribe_by_hash = (GdpMessage__CmdSubscribeByHash *)
						ep_mem_zalloc(sizeof *msg->cmd_subscribe_by_hash);
		gdp_message__cmd_subscribe_by_hash__init(
						msg->cmd_subscribe_by_hash);
		break;

	case GDP_ACK_CHANGED:
		msg->body_case = GDP_MESSAGE__BODY_ACK_CHANGED;
		msg->ack_changed = (GdpMessage__AckChanged *)
					ep_mem_zalloc(sizeof *msg->ack_changed);
		gdp_message__ack_changed__init(msg->ack_changed);
		break;

	case GDP_ACK_CONTENT:
		msg->body_case = GDP_MESSAGE__BODY_ACK_CONTENT;
		msg->ack_content = (GdpMessage__AckContent *)
					ep_mem_zalloc(sizeof *msg->ack_content);
		gdp_message__ack_content__init(msg->ack_content);
		msg->ack_content->dl = (GdpDatumList *)
					ep_mem_zalloc(sizeof (GdpDatumList));
		gdp_datum_list__init(msg->ack_content->dl);
		// individual datums need to be allocated and initialized when set
//		msg->ack_content->datum = (GdpDatum *)
//					ep_mem_zalloc(sizeof *msg->ack_content->datum);
//		gdp_datum__init(msg->ack_content->datum);
		break;

	case GDP_ACK_END_OF_RESULTS:
		msg->body_case = GDP_MESSAGE__BODY_ACK_END_OF_RESULTS;
		msg->ack_end_of_results = (GdpMessage__AckEndOfResults *)
					ep_mem_zalloc(sizeof *msg->ack_end_of_results);
		gdp_message__ack_end_of_results__init(msg->ack_end_of_results);
		break;

	default:
		if (cmd >= GDP_NAK_C_MIN && cmd <= GDP_NAK_S_MAX)
		{
			// other negative acknowledgement
			msg->body_case = GDP_MESSAGE__BODY_NAK;
			msg->nak = (GdpMessage__NakGeneric *)
						ep_mem_zalloc(sizeof *msg->nak);
			gdp_message__nak_generic__init(msg->nak);
		}
		else if (cmd >= GDP_ACK_MIN && cmd <= GDP_ACK_MAX)
		{
			// other positive acknowledgement
			msg->body_case = GDP_MESSAGE__BODY_ACK_SUCCESS;
			msg->ack_success = (GdpMessage__AckSuccess *)
						ep_mem_zalloc(sizeof *msg->ack_success);
			gdp_message__ack_success__init(msg->ack_success);
			break;
		}
		else
		{
			msg->body_case = GDP_MESSAGE__BODY__NOT_SET;
		}
		break;
	}

	ep_dbg_cprintf(Dbg, 31, "_gdp_msg_new => %p\n", msg);
	return msg;
}


void
_gdp_msg_free(gdp_msg_t **pmsg)
{
	ep_dbg_cprintf(Dbg, 24, "_gdp_msg_free(%p)\n", *pmsg);
	gdp_message__free_unpacked(*pmsg, NULL);
	*pmsg = NULL;
}


static void
print_pb_ts(const GdpTimestamp *ts, FILE *fp)
{
	if (ts == NULL)
		fprintf(fp, "(none)");
	else if (ts->sec == EP_TIME_NOTIME)
		fprintf(fp, "(notime)");
	else
	{
		EP_TIME_SPEC tv;

		tv.tv_sec = ts->sec;
		tv.tv_nsec = ts->nsec;
		tv.tv_accuracy = ts->accuracy;
		ep_time_print(&tv, fp, EP_TIME_FMT_HUMAN | EP_TIME_FMT_SIGFIG6);
	}
}


static void
print_pb_datum(const GdpDatum *d, FILE *fp, int indent)
{
	fprintf(fp, "pbdatum@%p\n", d);
	fprintf(fp, "%srecno %" PRIgdp_recno ", ts ",
			_gdp_pr_indent(indent), d->recno);
	print_pb_ts(d->ts, fp);
	if (d->has_prevhash)
	{
		fprintf(fp, "\n%sprevhash[%zd]=",
				_gdp_pr_indent(indent), d->prevhash.len);
		ep_hexdump(d->prevhash.data, d->prevhash.len, fp, EP_HEXDUMP_TERSE, 0);
	}
	else
	{
		fprintf(fp, "\n%sprevhash=(none)", _gdp_pr_indent(indent));
	}
	if (d->sig == NULL)
		fprintf(fp, "\n%ssig=(none)", _gdp_pr_indent(indent));
	else
		fprintf(fp, "\n%ssig=%p [%zd]", _gdp_pr_indent(indent),
					d->sig, d->sig->sig.len);
	fprintf(fp, ", data[%zd]=", d->data.len);
	if (ep_dbg_test(Dbg, 42))
	{
		fprintf(fp, "\n");
		ep_hexdump(d->data.data, d->data.len, fp, EP_HEXDUMP_ASCII, 0);
	}
	else
	{
		fprintf(fp, "(omitted)\n");
	}
}


void
_gdp_msg_dump(const gdp_msg_t *msg, FILE *fp, int indent)
{
	gdp_pname_t pname;
	char ebuf[100];

	if (fp == NULL)
		fp = ep_dbg_getfile();
	flockfile(fp);
	indent++;
	fprintf(fp, "msg@%p: ", msg);
	if (msg == NULL)
	{
		fprintf(fp, "NULL\n");
		goto done;
	}
	fprintf(fp, "cmd=%d=%s, rid=",
			msg->cmd, _gdp_proto_cmd_name(msg->cmd));
	if (msg->rid == GDP_PDU_NO_RID)
		fprintf(fp, "(none)");
	else if (msg->rid == GDP_PDU_ANY_RID)
		fprintf(fp, "(any)");
	else
		fprintf(fp, "%" PRIgdp_rid, msg->rid);
	fprintf(fp, ", l5seqno=");
	if (msg->l5seqno == GDP_PDU_NO_L5SEQNO)
		fprintf(fp, "(none)");
	else
		fprintf(fp, "%" PRIgdp_l5seqno, msg->l5seqno);

	fprintf(fp, "\n%ssig@%p", _gdp_pr_indent(indent), msg->sig);
	if (msg->sig == NULL)
		fprintf(fp, " (none)");
	else
		fprintf(fp, " len=%zd,  data=%p",
					msg->sig->sig.len,
					msg->sig->sig.data);

	fprintf(fp, ", body=");
	switch (msg->body_case)
	{
		int dno;

	case GDP_MESSAGE__BODY__NOT_SET:
		fprintf(fp, "(not set)\n");
		break;

	case GDP_MESSAGE__BODY_CMD_CREATE:
		{
			char *logname = pname;
			if (msg->cmd_create->logname.data != NULL)
				gdp_printable_name(msg->cmd_create->logname.data, pname);
			else
				logname = "(NULL)";
			fprintf(fp, "cmd_create:\n%slogname=%s\n%smetadata=",
					_gdp_pr_indent(indent), logname,
					_gdp_pr_indent(indent + 1));
			if (!ep_dbg_test(Dbg, 42))
				fprintf(fp, "(omitted)\n");
			else
			{
				fprintf(fp, "\n");
				ep_hexdump(msg->cmd_create->metadata->data.data,
							msg->cmd_create->metadata->data.len,
							fp, EP_HEXDUMP_ASCII, 0);
			}
		}
		break;

	case GDP_MESSAGE__BODY_CMD_OPEN:
		fprintf(fp, "cmd_open\n");
		break;

	case GDP_MESSAGE__BODY_CMD_CLOSE:
		fprintf(fp, "cmd_close\n");
		break;

	case GDP_MESSAGE__BODY_CMD_APPEND:
		fprintf(fp, "cmd_append: ndatums %zd\n", msg->cmd_append->dl->n_d);
		for (dno = 0; dno < msg->cmd_append->dl->n_d; dno++)
		{
			fprintf(fp, "%s[%d] ", _gdp_pr_indent(indent), dno);
			print_pb_datum(msg->cmd_append->dl->d[dno], fp, indent + 1);
		}
		break;

	case GDP_MESSAGE__BODY_CMD_READ_BY_RECNO:
		fprintf(fp, "cmd_read_by_recno:\n%srecno %" PRIgdp_recno,
				_gdp_pr_indent(indent), msg->cmd_read_by_recno->recno);
		if (msg->cmd_read_by_recno->has_nrecs)
			fprintf(fp, ", nrecs %"PRId32, msg->cmd_read_by_recno->nrecs);
		fprintf(fp, "\n");
		break;

	case GDP_MESSAGE__BODY_CMD_READ_BY_TS:
		fprintf(fp, "cmd_read_by_ts: ");
		print_pb_ts(msg->cmd_read_by_ts->timestamp, fp);
		if (msg->cmd_read_by_ts->has_nrecs)
			fprintf(fp, ", nrecs %"PRId32, msg->cmd_read_by_ts->nrecs);
		fprintf(fp, "\n");
		break;

	case GDP_MESSAGE__BODY_CMD_READ_BY_HASH:
		fprintf(fp, "cmd_read_by_hash: (printing unimplemented)\n");
		break;

	case GDP_MESSAGE__BODY_CMD_SUBSCRIBE_BY_RECNO:
		fprintf(fp, "cmd_subscribe_by_recno:\n"
					"%sstart %"PRIgdp_recno " nrecs %"PRId32 "\n",
					_gdp_pr_indent(indent),
					msg->cmd_subscribe_by_recno->start,
					msg->cmd_subscribe_by_recno->nrecs);
		break;

	case GDP_MESSAGE__BODY_CMD_SUBSCRIBE_BY_TS:
		fprintf(fp, "cmd_subscribe_by_hash: (printing unimplemented)\n");
		break;

	case GDP_MESSAGE__BODY_CMD_SUBSCRIBE_BY_HASH:
		fprintf(fp, "cmd_subscribe_by_hash: (printing unimplemented)\n");
		break;

	case GDP_MESSAGE__BODY_CMD_UNSUBSCRIBE:
		fprintf(fp, "cmd_unsubscribe: (no payload)\n");
		break;

	case GDP_MESSAGE__BODY_CMD_GET_METADATA:
		fprintf(fp, "cmd_get_metadata: (no payload)\n");
		break;

	case GDP_MESSAGE__BODY_CMD_DELETE:
		fprintf(fp, "cmd_delete: (no payload)\n");
		break;

	case GDP_MESSAGE__BODY_ACK_SUCCESS:
		fprintf(fp, "ack_success:\n%srecno ", _gdp_pr_indent(indent));
		if (msg->ack_success->has_recno)
			fprintf(fp, "%" PRIgdp_recno, msg->ack_success->recno);
		else
			fprintf(fp, "(none)");
		fprintf(fp, ", ts=");
		print_pb_ts(msg->ack_success->ts, fp);
		fprintf(fp, "\n");
		if (msg->ack_success->has_hash)
		{
			fprintf(fp, "%shash ", _gdp_pr_indent(indent + 1));
			ep_hexdump(msg->ack_success->hash.data,
						msg->ack_success->hash.len,
						fp, EP_HEXDUMP_TERSE, 0);
		}
		fprintf(fp, "%smetadata ", _gdp_pr_indent(indent + 1));
		if (!msg->ack_success->has_metadata)
			fprintf(fp, "(none)\n");
		else if (!ep_dbg_test(Dbg, 42))
			fprintf(fp, "(omitted)\n");
		else
		{
			fprintf(fp, "\n");
			ep_hexdump(msg->ack_success->metadata.data,
						msg->ack_success->metadata.len,
						fp, EP_HEXDUMP_ASCII, 0);
		}
		break;

	case GDP_MESSAGE__BODY_ACK_CHANGED:
		fprintf(fp, "ack_changed\n");
		break;

	case GDP_MESSAGE__BODY_ACK_CONTENT:
		fprintf(fp, "ack_content: ");
		for (dno = 0; dno < msg->ack_content->dl->n_d; dno++)
		{
			fprintf(fp, "[%d] ", dno);
			print_pb_datum(msg->ack_content->dl->d[dno], fp, indent + 1);
		}
		break;

	case GDP_MESSAGE__BODY_ACK_END_OF_RESULTS:
		fprintf(fp, "ack_end_of_results: nresults ");
		if (msg->ack_end_of_results->has_nresults)
			fprintf(fp, "%" PRIu64 "\n", msg->ack_end_of_results->nresults);
		else
			fprintf(fp, "(unset)\n");
		if (msg->nak->has_ep_stat)
			fprintf(fp, "%sep_stat %s\n",
					_gdp_pr_indent(indent),
					ep_stat_tostr(EP_STAT_FROM_INT(msg->nak->ep_stat),
								ebuf, sizeof ebuf));
		break;

	case GDP_MESSAGE__BODY_NAK:
		fprintf(fp, "nak:\n");
		if (msg->nak->has_ep_stat)
			fprintf(fp, "%sep_stat %s\n",
					_gdp_pr_indent(indent),
					ep_stat_tostr(EP_STAT_FROM_INT(msg->nak->ep_stat),
								ebuf, sizeof ebuf));
		if (msg->nak->description != NULL)
			fprintf(fp, "%sdetail %s\n",
					_gdp_pr_indent(indent),
					msg->nak->description);
		if (msg->nak->has_recno)
			fprintf(fp, "%srecno %" PRIgdp_recno "\n",
					_gdp_pr_indent(indent),
					msg->nak->recno);
		break;

	case GDP_MESSAGE__BODY_NAK_CONFLICT:
		fprintf(fp, "nak_conflict\n");
		if (msg->nak_conflict->has_ep_stat)
			fprintf(fp, "%sep_stat %s\n",
					_gdp_pr_indent(indent),
					ep_stat_tostr(EP_STAT_FROM_INT(msg->nak_conflict->ep_stat),
								ebuf, sizeof ebuf));
		if (msg->nak_conflict->description != NULL)
			fprintf(fp, "%sdetail %s\n",
					_gdp_pr_indent(indent),
					msg->nak_conflict->description);
		if (msg->nak_conflict->has_recno)
			fprintf(fp, "%srecno %" PRIgdp_recno "\n",
					_gdp_pr_indent(indent),
					msg->nak_conflict->recno);
		break;

	default:
		fprintf(fp, "unknown body case %d\n", msg->body_case);
		break;
	}

done:
	funlockfile(fp);
}
