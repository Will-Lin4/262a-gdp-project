/* vim: set ai sw=4 sts=4 ts=4 : */

/*
**  Log Advertisements
**
**	----- BEGIN LICENSE BLOCK -----
**	GDPLOGD: Log Daemon for the Global Data Plane
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


#include "logd.h"

#include <gdp/gdp.h>
#include <gdp/gdp_chan.h>
#include <gdp/gdp_priv.h>

#include <ep/ep_dbg.h>


static EP_DBG	Dbg = EP_DBG_INIT("gdplogd.advertise",
							"GDP GCL Advertisements");


typedef struct gdp_advert_x	gdp_advert_x_t;

struct gdp_advert_x
{
	gdp_chan_t				*chan;
	gdp_adcert_t			*adcert;
	gdp_chan_advert_cr_t	*challenge_cb;
};


static long		AdvertDelay;	// time between adverts (in microseconds)

void
logd_advertise_init(void)
{
	AdvertDelay = ep_adm_getlongparam("swarm.gdplogd.advertise.delay", 0);
}


/*
**  Advertise a new GCL
*/

void
logd_advertise_one(gdp_chan_t *chan, gdp_name_t gname, int cmd)
{
	EP_STAT estat;
	gdp_adcert_t *adcert = NULL;					//XXX XXX
	gdp_chan_advert_cr_t *challenge_cb = NULL;		//XXX XXX
	gdp_advert_x_t advert;

	memset(&advert, 0, sizeof advert);
	advert.chan = chan;

	if (cmd == GDP_CMD_ADVERTISE)
		estat = _gdp_chan_advertise(chan, gname, adcert, challenge_cb, &advert);
	else
		estat = _gdp_chan_withdraw(chan, gname, &advert);

	if (ep_dbg_test(Dbg, 11))
	{
		char ebuf[100];
		gdp_pname_t pname;

		ep_dbg_printf("logd_advertise_one(%s) => %s\n",
				gdp_printable_name(gname, pname),
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
}


/*
**  Advertise all known GCLs
*/

static EP_STAT
advertise_one(gdp_name_t gname, void *ax_)
{
	gdp_advert_x_t *ax = (gdp_advert_x_t *) ax_;
	EP_STAT estat;

	estat = _gdp_chan_advertise(ax->chan, gname, ax->adcert,
							ax->challenge_cb, ax);
	if (EP_STAT_ISOK(estat))
		estat = _gdp_chan_flush(ax->chan);
	if (AdvertDelay > 0)
		ep_time_nanosleep(AdvertDelay MICROSECONDS);

	if (ep_dbg_test(Dbg, 54))
	{
		gdp_pname_t pname;
		char ebuf[100];

		ep_dbg_printf("\tAdvertise %s => %s\n",
				gdp_printable_name(gname, pname),
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	return estat;
}

static EP_STAT
withdraw_one(gdp_name_t gname, void *ax_)
{
	gdp_advert_x_t *ax = (gdp_advert_x_t *) ax_;
	EP_STAT estat;

	if (!EP_ASSERT(ax != NULL))
		return EP_STAT_ASSERT_ABORT;

	estat = _gdp_chan_withdraw(ax->chan, gname, ax);
	if (EP_STAT_ISOK(estat))
		estat = _gdp_chan_flush(ax->chan);
	if (AdvertDelay > 0)
		ep_time_nanosleep(AdvertDelay MICROSECONDS);

	if (ep_dbg_test(Dbg, 54))
	{
		gdp_pname_t pname;
		char ebuf[100];

		ep_dbg_printf("\tWithdraw %s => %s\n",
				gdp_printable_name(gname, pname),
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	return estat;
}


EP_STAT
logd_advertise_all(gdp_chan_t *chan, int cmd, void *adata_unused)
{
	EP_STAT estat, tstat;
	gdp_adcert_t *adcert = NULL;					//XXX XXX
	gdp_chan_advert_cr_t *challenge_cb = NULL;		//XXX XXX
	gdp_advert_x_t advert;

	memset(&advert, 0, sizeof advert);
	advert.chan = chan;

	if (cmd == GDP_CMD_ADVERTISE)
	{
		ep_dbg_cprintf(Dbg, 24, "logd_advertise_all(ADVERTISE)\n");

		// advertise me ...
		estat = _gdp_chan_advertise(chan, _GdpMyRoutingName, adcert,
									challenge_cb, &advert);

		// ... and all of my logs
		tstat = GdpSqliteImpl.foreach(advertise_one, &advert);
	}
	else
	{
		ep_dbg_cprintf(Dbg, 24, "logd_advertise_all(WITHDRAW)\n");

		// withdraw log advertisements ...
		estat = GdpSqliteImpl.foreach(withdraw_one, &advert);

		// ... and finally myself
		tstat = _gdp_chan_withdraw(chan, _GdpMyRoutingName, &advert);
	}

	if (EP_STAT_SEVERITY(tstat) > EP_STAT_SEVERITY(estat))
		estat = tstat;
	tstat = _gdp_chan_advert_flush(chan);
	if (EP_STAT_SEVERITY(tstat) > EP_STAT_SEVERITY(estat))
		estat = tstat;

	if (ep_dbg_test(Dbg, 21))
	{
		char ebuf[100];

		ep_dbg_printf("logd_advertise_all => %s\n",
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	return estat;
}
