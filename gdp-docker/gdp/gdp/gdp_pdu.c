/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**	GDP_PDU.C --- low-level PDU internal <=> external translations
**
**		Everything is read and written in network byte order.
**
**		XXX Should we be working on raw files instead of stdio files?
**			Will make a difference if we use UDP.
**		XXX Should we pad out the PDU header to an even four bytes?
**			Might help some implementations when reading or writing
**			the data.
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
#include "gdp_pdu.h"
#include "gdp_priv.h"

#include <event2/event.h>

#include <string.h>
#include <sys/errno.h>

static EP_DBG	Dbg = EP_DBG_INIT("gdp.pdu", "GDP PDU manipulation");
static EP_DBG	DbgIn = EP_DBG_INIT("gdp.pdu.in", "GDP PDU incoming traffic");
static EP_DBG	DbgOut = EP_DBG_INIT("gdp.pdu.out", "GDP PDU outgoing traffic");


void
_gdp_pdu_dump(const gdp_pdu_t *pdu, FILE *fp, int indent)
{
	indent++;
	if (fp == NULL)
		fp = ep_dbg_getfile();
	flockfile(fp);
	fprintf(fp, "PDU@%p: ", pdu);
	if (pdu == NULL)
	{
		fprintf(fp, "NULL\n");
		goto done;
	}
	fprintf(fp, "\n%sdst=", _gdp_pr_indent(indent));
	gdp_print_name(pdu->dst, fp);
	fprintf(fp, "\n%ssrc=", _gdp_pr_indent(indent));
	gdp_print_name(pdu->src, fp);
	fprintf(fp, "\n%sseqno %d, ", _gdp_pr_indent(indent), pdu->seqno);
	_gdp_msg_dump(pdu->msg, fp, indent);

done:
	funlockfile(fp);
}


/*
**	GDP_PDU_OUT --- send a PDU to a network buffer
**
**		Outputs PDU, including all the data in the dbuf.
*/

EP_STAT
_gdp_pdu_out(gdp_pdu_t *pdu, gdp_chan_t *chan)
{
	EP_STAT estat = EP_STAT_OK;
	gdp_buf_t *obuf = NULL;
	int cmd = 0;

	EP_ASSERT_ELSE(pdu != NULL, return EP_STAT_ASSERT_ABORT);
	if (pdu->msg != NULL)
		cmd = pdu->msg->cmd;

	// avoid segfault if the daemon is not running
	if (chan == NULL)
	{
		ep_dbg_cprintf(Dbg, 1,
				"_gdp_pdu_out: no channel\n");
		return GDP_STAT_DEAD_DAEMON;
	}
	if (!gdp_name_is_valid(pdu->src))
	{
		// use our own name as the source if nothing specified
		memcpy(pdu->src, _GdpMyRoutingName, sizeof pdu->src);
	}

	pdu->chan = chan;
	obuf = gdp_buf_new();

	/*
	**  Protobuf should be ready to go now --- serialize it.
	**
	**		Currently uses evbuffer_reserve_space to avoid another
	**		copy.  Alternative: use evbuffer_add_reference.
	*/

	if (ep_dbg_test(DbgOut, 1))
		flockfile(ep_dbg_getfile());

	{
		size_t pb_len;
		int istat;
		struct evbuffer_iovec v[1];

		pdu->msg->has_rid = (pdu->msg->rid != GDP_PDU_NO_RID);
		pdu->msg->has_l5seqno = (pdu->msg->l5seqno != GDP_PDU_NO_L5SEQNO);

		// *__get_packed_size has no error returns
		pb_len = gdp_message__get_packed_size(pdu->msg);
		ep_dbg_cprintf(Dbg, 24,
				"_gdp_pdu_out: serialized length %zd, obuf len %zd\n",
				pb_len, gdp_buf_getlength(obuf));
		istat =  evbuffer_reserve_space(obuf, pb_len, v, 1);
		if (istat <= 0)
		{
			ep_dbg_cprintf(Dbg, 1,
					"_gdp_pdu_out: evbuffer_reserve_space(%zd) => %d\n",
					pb_len, istat);
			estat = GDP_STAT_PDU_WRITE_FAIL;
			goto fail1;
		}
		if (v[0].iov_len < pb_len)
		{
			ep_dbg_cprintf(Dbg, 1,
					"_gdp_pdu_out: cannot reserve space, need %zd, got %zd\n",
					pb_len, v[0].iov_len);
			estat = GDP_STAT_PDU_WRITE_FAIL;
			goto fail1;
		}

		pb_len = gdp_message__pack(pdu->msg, (uint8_t*) v[0].iov_base);
		v[0].iov_len = pb_len;
		istat = evbuffer_commit_space(obuf, v, 1);
		if (istat < 0)
		{
			ep_dbg_cprintf(Dbg, 1,
					"_gdp_pdu_out: cannot commit space\n");
			estat = GDP_STAT_PDU_WRITE_FAIL;
			goto fail1;
		}
		ep_dbg_cprintf(Dbg, 24,
				"_gdp_pdu_out: new obuf len %zd\n", gdp_buf_getlength(obuf));
	}

	if (ep_dbg_test(DbgOut, 18))
	{
		ep_dbg_printf("_gdp_pdu_out, chan = %p:", chan);
		if (ep_dbg_test(DbgOut, 22))
		{
			ep_dbg_printf("\n");
			// remainder will be printed below
		}
		else
		{
			ep_dbg_printf(" %s\n", _gdp_proto_cmd_name(cmd));
		}
	}

	if (ep_dbg_test(DbgOut, 22))
	{
		ep_dbg_printf("%s", _gdp_pr_indent(1));
		_gdp_pdu_dump(pdu, ep_dbg_getfile(), 1);
	}

fail1:
	if (ep_dbg_test(DbgOut, 1))
		funlockfile(ep_dbg_getfile());
	if (EP_STAT_ISOK(estat))
	{
		// actually send this all to the channel
		_gdp_chan_send(chan, NULL, pdu->src, pdu->dst, obuf,
					GDP_PKT_TYPE_REGULAR);
	}

//fail0:
	if (obuf != NULL)
		gdp_buf_free(obuf);

	return estat;
}


/*
**	GDP_PDU_IN --- read a PDU from the network
**
**		The PDU needs to have been initialized (using _gdp_pdu_new)
**		with everything except the message content (i.e., the
**		payload).
*/

EP_STAT
_gdp_pdu_in(
		gdp_pdu_t *pdu,			// PDU output area
		gdp_buf_t *pbuf,		// serialized (packed) proto buffer
		size_t plen,			// length of that buffer
		gdp_chan_t *chan)		// input channel
{
	EP_STAT estat = EP_STAT_OK;
	GdpMessage *msg = NULL;
	uint8_t *mbuf;

	if (!EP_ASSERT_POINTER_VALID(pdu) ||
		!EP_ASSERT(pdu->msg == NULL))
	{
		estat = EP_STAT_ASSERT_ABORT;
		goto fail1;
	}

	ep_dbg_cprintf(DbgIn, 30, "\n\t>>>>>  _gdp_pdu_in(%zd)  >>>>>\n", plen);

	// unpack Protobuf into local data structure
	mbuf = gdp_buf_getptr(pbuf, plen);
	msg = gdp_message__unpack(NULL, plen, mbuf);
	if (msg == NULL)
	{
		ep_dbg_cprintf(DbgIn, 1,
				"_gdp_pdu_in: error unpacking %zd octet message\n", plen);
		if (ep_dbg_test(DbgIn, 10))
			ep_hexdump(mbuf, plen, ep_dbg_getfile(), EP_HEXDUMP_ASCII, 0);
		estat = GDP_STAT_PDU_CORRUPT;
		goto fail1;
	}

	// XXX hack: cache chan into pdu for use when creating requests
	pdu->chan = chan;

	// decode Protobuf into gdp_pdu_t
	if (!msg->has_rid)
		msg->rid = GDP_PDU_NO_RID;
	if (!msg->has_l5seqno)
		msg->l5seqno = GDP_PDU_NO_L5SEQNO;

fail1:
	if (ep_dbg_test(DbgIn, 18))
	{
		char ebuf[200];
		const char *cmdname;

		if (msg == NULL)
			cmdname = "no message";
		else
			cmdname = _gdp_proto_cmd_name(msg->cmd),
		flockfile(ep_dbg_getfile());
		ep_dbg_printf("_gdp_pdu_in(%s) => %s\n", cmdname,
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
		if (ep_dbg_test(DbgIn, 22))
		{
			ep_dbg_printf("%s", _gdp_pr_indent(1));
			_gdp_pdu_dump(pdu, ep_dbg_getfile(), 1);
		}
		funlockfile(ep_dbg_getfile());
	}

	gdp_buf_drain(pbuf, plen);
	if (EP_STAT_ISOK(estat))
		pdu->msg = msg;
	else if (msg != NULL)
		gdp_message__free_unpacked(msg, NULL);
	return estat;
}


/*
**  _GDP_PDU_NEW --- allocate a PDU (from free list if possible)
**  _GDP_PDU_FREE --- return a PDU to the free list
*/

static EP_THR_MUTEX		PduFreeListMutex
							EP_THR_MUTEX_INITIALIZER2(GDP_MUTEX_LORDER_LEAF);
static TAILQ_HEAD(pkt_head, gdp_pdu)
						PduFreeList = TAILQ_HEAD_INITIALIZER (PduFreeList);

gdp_pdu_t *
_gdp_pdu_new(GdpMessage *msg, gdp_name_t src, gdp_name_t dst, gdp_seqno_t seqno)
{
	gdp_pdu_t *pdu;

	if (ep_dbg_test(Dbg, 48))
	{
		gdp_pname_t src_pname;
		gdp_pname_t dst_pname;
		ep_dbg_printf("_gdp_pdu_new(%d; %s => %s)\n",
					seqno,
					gdp_printable_name(src, src_pname),
					gdp_printable_name(dst, dst_pname));
	}

	ep_thr_mutex_lock(&PduFreeListMutex);
	do
	{
		if ((pdu = TAILQ_FIRST(&PduFreeList)) != NULL)
			TAILQ_REMOVE(&PduFreeList, pdu, list);
		else
		{
			pdu = (gdp_pdu_t *) ep_mem_zalloc(sizeof *pdu);
		}
		EP_ASSERT_ELSE(!EP_UT_BITSET(GDP_PDU_INUSE, pdu->flags), pdu = NULL);
	} while (pdu == NULL);
	ep_thr_mutex_unlock(&PduFreeListMutex);
	VALGRIND_HG_CLEAN_MEMORY(pdu, sizeof *pdu);

	// initialize the PDU
	memset(pdu, 0, sizeof *pdu);
	pdu->flags |= GDP_PDU_INUSE;
	pdu->msg = msg;
	memcpy(pdu->src, src, sizeof pdu->src);
	memcpy(pdu->dst, dst, sizeof pdu->dst);
	pdu->seqno = seqno;

	ep_dbg_cprintf(Dbg, 48, "_gdp_pdu_new => %p\n", pdu);
	return pdu;
}

void
_gdp_pdu_free(gdp_pdu_t **ppdu)
{
	gdp_pdu_t *pdu = *ppdu;
	ep_dbg_cprintf(Dbg, 48, "_gdp_pdu_free(%p)\n", pdu);

	// abandon this PDU if already free
	EP_ASSERT_ELSE(EP_UT_BITSET(GDP_PDU_INUSE, pdu->flags), return);
	pdu->flags &= ~GDP_PDU_INUSE;
	if (pdu->msg != NULL)
		_gdp_msg_free(&pdu->msg);
	*ppdu = NULL;
#if GDP_DEBUG_NO_FREE_LISTS		// avoid helgrind complaints
	ep_mem_free(pdu);
#else
	ep_thr_mutex_lock(&PduFreeListMutex); TAILQ_INSERT_HEAD(&PduFreeList, pdu, list);
	ep_thr_mutex_unlock(&PduFreeListMutex);
#endif
}
