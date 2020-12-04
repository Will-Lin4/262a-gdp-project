/* vim: set ai sw=4 sts=4 ts=4 : */

/*
**  Headers for Human-Oriented Names to GDPname Directory (HONGD).
**
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

/*
**	These headers are not intended for external use, but may be
**	useful in some internal utilities such as gdp-name-add(8).
*/

#ifndef _GDP_HONGD_H_
#define _GDP_HONGD_H_

#include <mysql.h>

#ifndef GDP_DEFAULT_HONGD_HOST
# define GDP_DEFAULT_HONGD_HOST			NULL	// must provide explicitly
#endif
#ifndef GDP_DEFAULT_HONGD_PROTO
# define GDP_DEFAULT_HONGD_PROTO		"tcp"
#endif
#ifndef GDP_DEFAULT_HONGD_USER
# define GDP_DEFAULT_HONGD_USER			"gdp_user"
#endif
#ifndef GDP_DEFAULT_HONGD_PASSWD
# define GDP_DEFAULT_HONGD_PASSWD		GDP_DEFAULT_HONGD_USER
#endif

struct hongdargs
{
	char		*name_root;			// root of names ("current directory")
	const char	*dbproto;			// protocol for accessing MariaDB
	const char	*dbhost;			// IP name of database host
	int			dbport;				// IP port of database host
	const char	*dbuser;			// database user name
	char		*dbpasswd;			// database user password
	const char	*dbname;			// database name
	const char	*dbtable;			// name of database mapping table
};


/*
**  Names
*/

EP_STAT			_gdp_name_init(				// initialize name resolution
					struct hongdargs *);
MYSQL			*_gdp_hongd_conn_open(		// open a MariaDB/MySQL connection
					struct hongdargs *);

#endif // _GDP_HONGD_H_
