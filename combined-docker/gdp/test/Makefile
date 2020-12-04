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

BINALL= \
		gdp-stresser \
		t_async_append \
		t_conn_pool \
		t_ep_time_format \
		t_ep_time_parse_interval \
		t_fwd_append \
		t_multimultiread \
		t_pcari_1 \
		t_sub_and_append \
		t_unsubscribe \

SBINALL=

MAN1ALL=

MAN3ALL=

MAN5ALL=

MAN7ALL=

MAN8ALL=

MANALL=		${MAN1ALL} ${MAN3ALL} ${MAN5ALL} ${MAN7ALL} ${MAN8ALL}

# files that should be removed on "make clean"
CLEANALL=	${BINALL} ${SBINALL}
# files that should be made public
PUBALL=		${SCRIPTS} ${MANALL}
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
INCGDP=		-I${INCROOT}
INCEP=
INCS=		${INCSEARCH} ${INCGDP} ${INCEP}

LIBROOT=	${INCROOT}
LIBSEARCH=	${LIBLOCAL} \
		-L${LIBROOT}/gdp \
		-L${LIBROOT}/ep \
		-L${LOCAL1}/lib \
		-L${LOCAL2}/lib \
		-L${LOCAL3}/lib
LIBGDP=		-lgdp
LIBEP=		-lep
LIBJANSSON=	-L${LOCAL1}/lib -L${LOCAL2}/lib -ljansson
INCEVENT2=
LIBEVENT2=	-levent -levent_pthreads -pthread
LIBCRYPTO=	-lcrypto
LIBPROTO_C=	-lprotobuf-c
LIBAVAHI=	-lavahi-client -lavahi-common
LIBADD=		`sh ../adm/add-libs.sh`
LDLIBS=		${LIBGDP} \
		${LIBEP} \
		${LIBEVENT2}  \
		${LIBCRYPTO}  \
		${LIBPROTO_C} \
		${LIBAVAHI} \
		${LIBJANSSON}  \
		${LIBADD}
LDFLAGS+=	t_common_support.o \
		${LIBSEARCH} \
		${LDLIBS} \
		${SANITIZE}
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
	@echo "To actually run the tests, run make allTests"


${CLEANALL}:	${LIBROOT}/gdp/libgdp.a ${LIBROOT}/ep/libep.a \
		t_common_support.o

clean:
	-rm -f ${CLEANALL} *.o *.core

${ALLDIRS}:
	${MKDIR} $@

run: all
	./run.sh

allTests: all
	-(cd ../lang/python/test; ./run.sh)
	-./run.sh

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
