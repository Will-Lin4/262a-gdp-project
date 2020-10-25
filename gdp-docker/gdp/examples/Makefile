#
#  ----- BEGIN LICENSE BLOCK -----
#	GDP: Global Data Plane
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

ALL=		machine-mon

DESTDIR=
LOCALROOT=	/usr
INSTALLROOT=	${DESTDIR}${LOCALROOT}
BINDIR=		${INSTALLROOT}/bin
SBINDIR=	${INSTALLROOT}/sbin
ALLDIRS=	${BINDIR} \
		${SBINDIR} \
		${LIBDIR} \
		${INCDIR}/ep \
		${MANDIR}/man1 \
		${MANDIR}/man3 \
		${MANDIR}/man5 \
		${MANDIR}/man7 \
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
		-I${LOCAL1}/include \
		-I${LOCAL2}/include \
		-I${LOCAL3}/include \
		`mysql_config --cflags`
LIBROOT=	${INCROOT}
LIBSEARCH=	${LIBLOCAL} \
		-L${LIBROOT}/gdp \
		-L${LIBROOT}/ep \
		-I${LOCAL1}/lib \
		-L${LOCAL2}/lib \
		-L${LOCAL3}/lib
LIBGDP=		-lgdp
LIBEP=		-lep
LIBJANSSON=	-L${LOCAL1}/lib -L${LOCAL2}/lib -ljansson
LIBEVENT2=	-levent -levent_pthreads -pthread
LIBCRYPTO=	-lcrypto
LIBPROTO_C=	-lprotobuf-c
LIBAVAHI=	-lavahi-client -lavahi-common
LIBADD=		`sh ../adm/add-libs.sh`
LDLIBS=		${LIBGDP} \
		${LIBEP} \
		${LIBEVENT2} \
		${LIBCRYPTO} \
		${LIBPROTO_C} \
		${LIBAVAHI} \
		${LIBJANSSON} \
		${LIBADD}
INCS=		${INCSEARCH} ${INCGDP} ${INCEP}
LDFLAGS+=	${LIBSEARCH} ${LDLIBS} ${SANITIZE}
CC=		cc
PG=
WALL=		-Wall
G=		-g
O=		-O
STD=
COPTS=		${PG} ${WALL} $G $O ${STD} ${SANITIZE}
CFLAGS+=	${COPTS} ${INCS}
MKDIR=		mkdir -p

all:	${ALL}

# Build without avahi, the zero-conf facility that
# can be tricky to compile under Mac OS X.
all_noavahi:
	${MAKE}	STD=-DGDP_OSCF_USE_ZEROCONF=0 LIBAVAHI=  all

machine-mon:	machine-mon.o
	${CC} -o $@ machine-mon.o ${LDFLAGS}

clean:
	-rm -f ${ALL} *.o *.core

${ALLDIRS}:
	${MKDIR} $@

ADM=		../adm
UPDATE_LICENSE=	${ADM}/update-license.sh

update-license:
	${UPDATE_LICENSE} Makefile *.[ch]
