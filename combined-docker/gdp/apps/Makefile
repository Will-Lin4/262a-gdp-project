#
#  ----- BEGIN LICENSE BLOCK -----
#	Applications for the Global Data Plane
#	From the Ubiquitous Swarm Lab, 490 Cory Hall, U.C. Berkeley.
#
#	Copyright (c) 2015-2019, Regents of the University of California.
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

BINALL_ZC=	gdp-zcbrowse
BINALL=		\
		log-exists \
		log-mirror \
		gdp-create \
		gdp-reader \
		gdp-writer \
		$(BINALL_ZC) \

SBINALL_ZC= 	gdp-zcpublish
SBINALL=	\
		gdp-find-xname \
		gdp-name-add \
		gdp-name-xlate \
		gdp-delete \
		gdp-log-view \
		$(SBINALL_ZC) \
		gdp-rest \
#		gdp-log-check \

MAN1ALL=	gdp-reader.1 \
		gdp-writer.1 \
		log-exists.1 \
		log-mirror.1 \

MAN3ALL=

MAN5ALL=

MAN7ALL=

MAN8ALL=	\
		gdp-create.8 \
		gdp-delete.8 \
		gdp-log-view.8 \
		gdp-name-add.8 \
#		gdp-log-check.8 \
#		gcl-clone.8 \

MANALL=		${MAN1ALL} ${MAN3ALL} ${MAN5ALL} ${MAN7ALL} ${MAN8ALL}

# files that should be removed on "make clean"
CLEANALL=	${BINALL} ${SBINALL}
# files that should be made public
PUBALL=		${SCRIPTS} ${BINALL} ${SBINALL} ${MANALL}
# "make all" targets (may be overlaps)
ALL=		${CLEANALL} ${PUBALL}

DESTDIR=
LOCALROOT=	/usr
INSTALLROOT=	${DESTDIR}${LOCALROOT}
BINDIR=		${INSTALLROOT}/bin
SBINDIR=	${INSTALLROOT}/sbin
MANDIR=		${INSTALLROOT}/share/man
ALLDIRS=	${BINDIR} \
		${SBINDIR} \
		${MANDIR}/man1 \
		${MANDIR}/man8

INSTALLFLAGS=
INSTALL=	install ${INSTALLFLAGS}
CP=		cp

LOCAL1=		/usr/local
LOCAL2=		/opt/local
LOCAL3=		/usr/local/opt/openssl
INCROOT=	..
INCSEARCH=	${INCLOCAL} \
		-I${INCROOT} \
		`mysql_config --include` \
		-I${LOCAL1}/include \
		-I${LOCAL2}/include \
		-I${LOCAL3}/include
LIBROOT=	${INCROOT}
LIBSEARCH=	${LIBLOCAL} \
		-L${LIBROOT}/gdp \
		-L${LIBROOT}/ep \
		-L${LOCAL1}/lib \
		-L${LOCAL2}/lib \
		-L${LOCAL3}/lib
INCGDP=		-I${INCROOT}
LIBGDP=		-lgdp
INCEP=
LIBEP=		-lep
LIBJANSSON=	-ljansson
LIBEVENT2=	-levent -levent_pthreads -pthread
LIBCRYPTO=	-lcrypto
LIBPROTO_C=	-lprotobuf-c
LIBMYSQL=	`mysql_config --libs`
LIBSQLITE=	-lsqlite3
LIBADD=		`sh ../adm/add-libs.sh`
INCS=		${INCSEARCH} ${INCGDP} ${INCEP}
LDFLAGS+=	${LIBSEARCH} ${SANITIZE} ${LDADD} ${PG}
LDLIBS=		${LIBGDP} \
		${LIBEP} \
		${LIBEVENT2} \
		${LIBCRYPTO} \
		${LIBPROTO_C} \
		${LIBJANSSON} \
		${LIBMYSQL} \
		${LIBADD}
PG=
WALL=		-Wall
G=		-g
O=		-O
STD=
COPTS=		${PG} ${WALL} $G $O ${STD} ${SANITIZE}
CFLAGS+=	${COPTS} ${INCS} ${CCADD}
MKDIR=		mkdir -p

all:	${ALL}

# Build without avahi, the zero-conf facility that
# can be tricky to compile under Mac OS X.
all_noavahi:
	${MAKE}	STD=-DGDP_OSCF_USE_ZEROCONF=0 LIBAVAHI= BINALL_ZC= SBINALL_ZC= all

gdp-log-check.o: ../gdplogd/logd_disklog.c ../gdplogd/logd_gcl.c

gdp-log-view: gdp-log-view.o
	${CC} ${INCS} -o $@ ${LDFLAGS} gdp-log-view.o ${LDLIBS} ${LIBSQLITE}

gdp-log-view.o: gdp-log-view.c ../gdplogd/logd_sqlite.c ../gdp/gdp_hongd.h

gdp-name-add.o: gdp-name-add.c ../gdp/gdp_hongd.h
	${CC} -c -o $@ ${CFLAGS} `mysql_config --cflags` gdp-name-add.c

gdp-name-xlate.o: ../gdp/gdp_hongd.h

gdp-rest:	gdp-rest.c ../scgilib/scgilib.o
	${CC} ${INCS} -o $@ ${LDFLAGS} gdp-rest.c ${LDLIBS} ../scgilib/scgilib.o

${CLEANALL}:	${LIBROOT}/gdp/libgdp.a ${LIBROOT}/ep/libep.a

clean:
	-rm -f ${CLEANALL} *.o *.core

install:	${ALLDIRS} ${PUBALL} FORCE
	test -z "${BINALL}"  || ${INSTALL} ${BINALL}  ${BINDIR}
	test -z "${SBINALL}" || ${INSTALL} ${SBINALL} ${SBINDIR}
	test -z "${MAN1ALL}" || ${CP} ${MAN1ALL} ${MANDIR}/man1
	test -z "${MAN3ALL}" || ${CP} ${MAN3ALL} ${MANDIR}/man3
	test -z "${MAN5ALL}" || ${CP} ${MAN5ALL} ${MANDIR}/man5
	test -z "${MAN7ALL}" || ${CP} ${MAN7ALL} ${MANDIR}/man7
	test -z "${MAN8ALL}" || ${CP} ${MAN8ALL} ${MANDIR}/man8

${ALLDIRS}:
	${MKDIR} $@

ADM=		../adm
UPDATE_LICENSE=	${ADM}/update-license.sh

update-license:
	${UPDATE_LICENSE} Makefile *.[ch]

# Redefine the implicit compile rule; GNU make changed the rule slightly
# which causes explicit paths to libgdp.a and libep.a to be added to
# the command, which in turn breaks compiling under C++.
.c:
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $< ${LDLIBS}

FORCE:
