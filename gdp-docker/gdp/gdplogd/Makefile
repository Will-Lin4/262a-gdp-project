#
#  ----- BEGIN LICENSE BLOCK -----
#	GDPLOGD: Log Daemon for the Global Data Plane
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

# temporarily "gdplogd2" during transition, eventually "gdplogd"	XXX
VER=
GDPLOGD=	gdplogd${VER}

BINALL=

SBINALL=	${GDPLOGD}

SCRIPTS=

MAN1ALL=

MAN3ALL=

MAN5ALL=

MAN7ALL=

MAN8ALL=	gdplogd.8

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
		${LIBDIR} \
		${MANDIR}/man8

INSTALLFLAGS=
INSTALL=	install ${INSTALLFLAGS}

LOCAL1=		/usr/local
LOCAL2=		/opt/local
LOCAL3=		/usr/local/opt/openssl
SANITIZE=
#SANITIZE+=	-fsanitize=address
#SANITIZE+=	-fsanitize=thread
#SANITIZE+=	-fsanitize=memory
#SANITIZE+=	-fsanitize=undefined

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
INCEP=		-I${INCROOT}
LIBEP=		-lep
LIBEVENT2=	-levent -levent_pthreads -pthread
LIBCRYPTO=	-lcrypto
LIBPROTO_C=	-lprotobuf-c
LIBAVAHI=	-lavahi-client -lavahi-common
LIBSQLITE=	-lsqlite3
LIBMYSQL=	`mysql_config --libs`
LIBADD=		`sh ../adm/add-libs.sh gdplogd`
INCS=		${INCSEARCH} ${INCGDP} ${INCEP}
LDFLAGS+=	${LIBSEARCH} ${SANITIZE} ${LDADD} ${PG}
LDLIBS=		${LIBGDP} \
		${LIBEP} \
		${LIBEVENT2} \
		${LIBCRYPTO} \
		${LIBPROTO_C} \
		${LIBAVAHI} \
		${LIBSQLITE} \
		${LIBMYSQL} \
		${LIBADD}
CC=		cc
PG=
WALL=		-Wall
G=		-g
O=		-O
STD=
COPTS=		${PG} ${WALL} $G $O ${STD} ${SANITIZE}
CFLAGS+=	${COPTS} ${INCS} ${CCADD}
MKDIR=		mkdir -p

OBJS=	\
		logd.o \
		logd_admin.o \
		logd_adv.o \
		logd_sqlite.o \
		logd_gcl.o \
		logd_proto.o \
		logd_pubsub.o \
		logd_version.o \

HDEPS=	\
		logd.h \
		logd_admin.h \
		logd_sqlite.h \
		logd_pubsub.h \
		${INCROOT}/gdp/gdp.h \
		${INCROOT}/gdp/gdp_chan.h \
		${INCROOT}/gdp/gdp_pdu.h \

LIBDEPS= \
		${LIBROOT}/gdp/libgdp.a \
		${LIBROOT}/ep/libep.a \

all: ${ALL}

# Build without avahi, the zero-conf facility that
# can be tricky to compile under Mac OS X.
all_noavahi:
	${MAKE}	STD=-DGDP_OSCF_USE_ZEROCONF=0 LIBAVAHI= OBJS_ZC= all

logd_version.o: logd_version.c ${OBJS_MOST} Makefile
	${CC} ${CFLAGS} \
		-D_VERSION_INFO_="\"`sh ../adm/get-version-info.sh`\"" \
		-c logd_version.c

${GDPLOGD}: ${OBJS} ${LIBDEPS}
	${CC} -o $@ ${LDFLAGS} ${OBJS} ${LDLIBS}

clean:
	-rm -f ${CLEANALL} *.o *.core

install:	install-check install-override

install-check:
	@if [ -d /etc/systemd/system -a ! -f /usr/include/systemd/sd-daemon.h ];\
	then\
		echo "Warning: You seem to be using systemd, but package systemd-dev not installed.";\
		echo "         System startup may not work.";\
		echo "         Use \"make install-override\" to ignore this message.";\
		exit 1;\
	fi

install-override:	${ALLDIRS} ${PUBALL}
	test -z "${BINALL}"  || ${INSTALL} ${BINALL}  ${BINDIR}
	test -z "${SBINALL}" || ${INSTALL} ${SBINALL} ${SBINDIR}
	test -z "${MAN1ALL}" || ${INSTALL} ${MAN1ALL} ${MANDIR}/man1
	test -z "${MAN3ALL}" || ${INSTALL} ${MAN3ALL} ${MANDIR}/man3
	test -z "${MAN5ALL}" || ${INSTALL} ${MAN5ALL} ${MANDIR}/man5
	test -z "${MAN7ALL}" || ${INSTALL} ${MAN7ALL} ${MANDIR}/man7
	test -z "${MAN8ALL}" || ${INSTALL} ${MAN8ALL} ${MANDIR}/man8
	cd ../adm && make VER=${VER} install-gdplogd-wrapper

${ALLDIRS}:
	${MKDIR} $@

${OBJS}: ${HDEPS}

${ALL}: ${LIBDEPS}

ADM=		../adm
UPDATE_LICENSE=	${ADM}/update-license.sh

update-license:
	${UPDATE_LICENSE} Makefile *.[ch]
