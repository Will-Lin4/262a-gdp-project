#
#  ----- BEGIN LICENSE BLOCK -----
#	GDP: Global Data Plane Support Library
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

# Version number of library.  All others are derived from here.
# NOTE: lang/python/gdp/MISC.py has MAJOR.MINOR hard coded; if you change
#       either of them, change `gdp_ver` in that file too.  (This is not
#       a feature.)
# FIXME: This should be in the top level Makefile, not here.
GDP_VERSION_MAJOR=	2
GDP_VERSION_MINOR=	3
GDP_VERSION_PATCH=	3

# These are generally changed on make command line
DESTDIR=
LOCALROOT=	/usr
LOCAL1=		/usr/local
LOCAL2=		/opt/local
LOCAL3=		/usr/local/opt/openssl

# These generally do not need to be changed
INSTALLROOT=	${DESTDIR}${LOCALROOT}
INCDIR=		${INSTALLROOT}/include
LIBDIR=		${INSTALLROOT}/lib
BINDIR=		${INSTALLROOT}/bin
SBINDIR=	${INSTALLROOT}/sbin
MANDIR=		${INSTALLROOT}/share/man
ALLDIRS=	${BINDIR} \
		${SBINDIR} \
		${LIBDIR} \
		${INCDIR}/gdp \
		${MANDIR}/man7 \
		${MANDIR}/man8
ADM=		../adm
LOCALLIBSDIR=	../libs
LIBNAME=	libgdp

BINALL=

SBINALL=

MAN1ALL=

MAN3ALL=

MAN5ALL=

MAN7ALL=	gdp.7 \

MAN8ALL=

MANALL=		${MAN1ALL} ${MAN3ALL} ${MAN5ALL} ${MAN7ALL} ${MAN8ALL}

OBJS_ZC = \
	gdp_zc_client.o \
	gdp_zc_server.o

OBJS_MOST=	\
	gdp_api.o \
	gdp_buf.o \
	gdp_chan.o \
	gdp_create.o \
	gdp_crypto.o \
	gdp_datum.o \
	gdp_event.o \
	gdp_gob_cache.o \
	gdp_gob_mgmt.o \
	gdp_gob_ops.o \
	gdp_main.o \
	gdp_md.o \
	gdp_msg.o \
	gdp_name.o \
	gdp_pdu.o \
	gdp_proto.o \
	gdp_req.o \
	gdp_stat.o \
	gdp_subscr.o \
	gdp_util.o \
	gdp.pb-c.o \
	$(OBJS_ZC)

OBJS=	\
	${OBJS_MOST} \
	gdp_version.o

PUBHFILES=	\
	gdp.h \
	gdp_buf.h \
	gdp_stat.h \
	gdp_version.h \
	gdp_zc_client.h \
	gdp_zc_server.h \

PRIVHFILES=	\
	gdp_chan.h \
	gdp_event.h \
	gdp_hongd.h \
	gdp_md.h \
	gdp_pdu.h \
	gdp_priv.h \
	gdp.pb-c.h \

HFILES=		${PUBHFILES} ${PRIVHFILES}

GDPLIBVER=	${GDP_VERSION_MAJOR}.${GDP_VERSION_MINOR}
INCALL=		${PUBHFILES}
LIBALL=		libgdp.a libgdp.so.${GDPLIBVER}
# files that should be removed on "make clean"
CLEANALL=	${LIBALL} ${BINALL} ${SBINALL} gdp_version.h
# files that should be publicly installed
PUBALL=		${SCRIPTS} ${MANALL} ${INCALL} ${LIBALL}
# "make all" targets (may be overlap)
ALL=		${CLEANALL} ${PUBALL}

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
LIBEVENT2=	-levent -levent_pthreads
LIBCRYPTO=	-lcrypto
LIBPROTO_C=	-lprotobuf-c
LIBAVAHI=	-lavahi-client -lavahi-common
LIBMYSQL=	`mysql_config --libs`
LIBADD=		`sh ../adm/add-libs.sh libgdp`
INCEP=		-I${INCROOT}
LIBEP=		-lep
INCS=		${INCSEARCH} ${INCEP} ${INCEVENT2} ${INCAVAHI}
LIBSEARCH=	${LIBLOCAL} \
		-L${LIBROOT}/ep \
		-L${LOCAL1}/lib \
		-L${LOCAL2}/lib \
		-L${LOCAL3}/lib
LDFLAGS+= 	${LIBSEARCH} ${SANITIZE}
LDLIBS=		${LIBEP} \
		${LIBEVENT2} \
		${LIBCRYPTO} \
		${LIBPROTO_C} \
		${LIBAVAHI} \
		${LIBMYSQL} \
		${LIBADD}
CC=		cc
PG=
WALL=		-Wall
G=		-g
O=		-O
STD=
COPTS=		${PG} ${WALL} $G $O ${STD} ${SANITIZE}
CFLAGS+=	${COPTS} ${INCS} -fPIC

LN=		ln
MV=		mv
CP=		cp
RM=		rm
MKDIR=		mkdir -p
RANLIB=		ranlib
PROTOC_C=	protoc-c
SHARED=		-shared ${CRYPTOLFLAGS}
#SHARED=		-shared ${CRYPTOLFLAGS} -Wl,-soname,$@
INSTALLFLAGS=
INSTALL=	install ${INSTALLFLAGS}

all: ${ALL}

# Build without avahi, the zero-conf facility that
# can be tricky to compile under Mac OS X.
all_noavahi:
	${MAKE}	STD=-DGDP_OSCF_USE_ZEROCONF=0 LIBAVAHI= OBJS_ZC= all

${LIBNAME}.a: ${OBJS}
	${AR} -r $@ ${OBJS}
	${RANLIB} $@
	${RM} -f ${LOCALLIBSDIR}/$@
	${CP} $@ ${LOCALLIBSDIR}

${LIBNAME}.so.${GDPLIBVER}: ${OBJS}
	${CC} ${SHARED} -o $@ ${LDFLAGS} ${OBJS} ${LDLIBS}
	${RM} -f ${LOCALLIBSDIR}/$@
	${CP} $@ ${LOCALLIBSDIR}
	sh ${LOCALLIBSDIR}/makelinks.sh gdp ${GDP_VERSION_MAJOR} ${GDP_VERSION_MINOR} ${LOCALLIBSDIR}

${OBJS}: ${HFILES}


gdp.pb-c.c gdp.pb-c.h: gdp.proto
	${PROTOC_C} --c_out=. gdp.proto

gdp_version.o: gdp_version.c ${OBJS_MOST} Makefile gdp_version.h
	${CC} ${CFLAGS} \
		-D_VERSION_INFO_="\"`sh ../adm/get-version-info.sh`\"" \
		-c gdp_version.c

gdp_version.h ../adm/gdp-version.sh: gdp_version.template gdp_version.sh Makefile
	sh gdp_version.sh ${GDP_VERSION_MAJOR} ${GDP_VERSION_MINOR} ${GDP_VERSION_PATCH}

#../git-version.txt:
#	../adm/get-version-info.sh > /dev/null

#
#  Administrative stuff
#

# cleanup
clean:
	-rm -f ${CLEANALL} *.o *.core

# system installation
install-lib: ${LIBDIR} ${LIBALL}
	test -z "${LIBALL}"  || ${CP} ${LIBALL}  ${LIBDIR}
	sh ${LOCALLIBSDIR}/makelinks.sh gdp ${GDP_VERSION_MAJOR} ${GDP_VERSION_MINOR} ${LIBDIR}

install-include: ${INCDIR}/gdp ${INCALL}
	test -z "${INCALL}"  || ${CP} ${INCALL}  ${INCDIR}/gdp

install-bin: ${BINDIR} ${SBINDIR}
	test -z "${BINALL}"  || ${INSTALL} ${BINALL}  ${BINDIR}
	test -z "${SBINALL}" || ${INSTALL} ${SBINALL} ${SBINDIR}

install-man: ${MANDIR} ${MANALL}
	test -z "${MAN1ALL}" || ${CP} ${MAN1ALL} ${MANDIR}/man1
	test -z "${MAN3ALL}" || ${CP} ${MAN3ALL} ${MANDIR}/man3
	test -z "${MAN7ALL}" || ${CP} ${MAN7ALL} ${MANDIR}/man7
	test -z "${MAN8ALL}" || ${CP} ${MAN8ALL} ${MANDIR}/man8

install:	install-include install-lib install-bin install-man

${ALLDIRS}:
	${MKDIR} $@

# local installation (into source tree, needed for Java & Debian package build)
# XXX Why is this needed?  It's done when the libraries are built.
install-local:	${LOCALLIBSDIR} ${LIBALL}
	${CP} ${LIBALL} ${LOCALLIBSDIR}
	sh ${LOCALLIBSDIR}/makelinks.sh gdp ${GDP_VERSION_MAJOR} ${GDP_VERSION_MINOR} ${LOCALLIBSDIR}

UPDATE_LICENSE=	${ADM}/update-license.sh

update-license:
	${UPDATE_LICENSE} Makefile *.[ch]

FORCE:
