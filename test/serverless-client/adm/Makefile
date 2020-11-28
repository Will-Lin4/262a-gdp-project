#
#  ----- BEGIN LICENSE BLOCK -----
#	GDP: Global Data Plane administrative files
#	From the Ubiquitous Swarm Lab, 490 Cory Hall, U.C. Berkeley.
#
#	Copyright (c) 2015-2017, Regents of the University of California.
#	All rights reserved.
#
#	Permission is hereby granted, without written agreement and without
#	license or royalty fees, to use, copy, modify, and distribute this
#	software and its documentation for any purpose, provided that the above
#	copyright notice and the following two paragraphs appear in all copies
#	of this software.
#
#	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
#	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
#	PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
#	EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
#	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#	FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION,
#	IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO
#	OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS,
#	OR MODIFICATIONS.
#  ----- END LICENSE BLOCK -----
#

# DESTDIR is just for staging.  LOCALROOT should be /usr or /usr/local.
DESTDIR=
LOCALROOT=	/usr
INSTALLROOT=	${DESTDIR}${LOCALROOT}
DOCDIR=		${INSTALLROOT}/share/doc/gdp
# /usr/libexec on BSD and Mac, /usr/sbin on most Linux
LIBEXECDIR=	${INSTALLROOT}/sbin
VER=		2
INSTALLFLAGS=
INSTALL=	install ${INSTALLFLAGS}

# install gdplogd support files (called from gdplogd/Makefile)
install-gdplogd-wrapper: gdplogd-wrapper.sh
	${INSTALL} gdplogd-wrapper.sh ${LIBEXECDIR}/gdplogd${VER}-wrapper.sh

ADM=		../adm
UPDATE_LICENSE=	${ADM}/update-license.sh

update-license:
	${UPDATE_LICENSE} Makefile
	(cd ep;		 ${MAKE} update-license)
	(cd gdp;	 ${MAKE} update-license)
	(cd gdplogd;	 ${MAKE} update-license)
	(cd apps;	 ${MAKE} update-license)

DOC=		../doc
PANDOC=		pandoc
PANFLAGS=	-s -c ${DOC}/gdpdoc.css

.SUFFIXES: .md .html

.md.html:
	${PANDOC} ${PANFLAGS} -o $@ $<
