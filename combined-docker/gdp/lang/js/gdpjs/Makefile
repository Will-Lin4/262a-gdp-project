#
#  Makefile for libgdpjs  (based on gdp/libep/Makefile)

#	Copyright (c) 2008-2015, Eric P. Allman.  All rights reserved.
#	Copyright (c) 2014-2017, Regents of the University of California.
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
#	OR MODIFICATIONS.#
#
# Author: Alec Dara-Abrams, Christopher Brooks
# 2014-10-24
#
#
# Caveats:
#
#    This Makefile assumes that it is located in gdp/lang/js/gdpjs/ ,
#    where gdp/ is a GDP Git local repository.  See GDPROOT and
#    the (unstructured) -I..'s below.
#
#    It will not force a re-build of the system up in gdp/ .
#    gdp/ must be re-built and kept up to date separately.
#
#    Products of the build:
#      1) *.o and *.a are left in the current directory
#      2) the dynamic library, libgdpjs.so.<<major>>.<<minor>> is moved
#      to gdpjs/libs/ and variously named copies are sym linked to it there.
#    Note, some of the various dynamic library sym linked files have
#    version information within their file names.
#
#    Yes, this Makefile is not as clean, structured, and parameterized
#    as it could be!


# External makefile targets below: all, clean
# TBD: provide a test/ sub-directory.


# Internal makefile variables

CURDIR=	.

OBJS=	\
	gdpjs_supt.o \

HFILES=	\
	gdpjs_supt.h \

CC=	cc
PG=
O=	-O
WALL=	-Wall
STD=
COPTS=	${PG} -g ${WALL} $O ${STD}

LOCAL1=	/usr/local
LOCAL2=	/opt/local
# /opt/local/include is for Mac OS X.
CFLAGS=	${COPTS} ${INCLS} -I${CURDIR} -I.. -I../.. -I../../.. -I${LOCAL1}/include -I${LOCAL2}/include -fPIC
RM=	rm
LD=	ld
GDPROOT= ../../..
LIBROOT= ${GDPROOT}
# /opt/local/lib is for Mac OS X.
LDCRYPTO= -L/opt/local/lib -lcrypto 
LIBAVAHI= -lavahi-common -lavahi-client
LDEVENT2= -levent -levent_pthreads
LDEP=   -L${LIBROOT}/ep -lep
LDGDP=  -L${LIBROOT}/gdp -lgdp
#LDFLAGS=-L.
LDFLAGS= ${LDGDP} ${LDEP} ${LDEVENT2} ${LDCRYPTO} ${LIBAVAHI} -L${LOCAL1}/lib -L${LOCAL2}/lib
LDLIBS=	libgdpjs.a
LIBDIR= ../libs
LN=	ln
MV=	mv
RANLIB=	ranlib
SHARED=	-shared
#SHARED=	-shared -Wl,-soname,$@
TIME=

# Version of this library
GDPJSLIBMAJVER=	1
GDPJSLIBMINVER=	0

GDPJSLIBVER=	${GDPJSLIBMAJVER}.${GDPJSLIBMINVER}

# Version of the GDP Library, should match ../../../gdp/Makefile
GDPLIBMAJVER=	0
GDPLIBMINVER=	8

GDPLIBVER= 	${GDPLIBMAJVER}.${GDPLIBMINVER}
# The name of the library in ../../../gdp
LIBGDP=		libgdp-${GDPLIBVER}

# Version of the EP Library, should match ../../../ep/Makefile
EPLIBMAJVER=	3
EPLIBMINVER=	0

ALL=		libgdpjs.a libgdpjs.so.${GDPJSLIBVER}

all: ${ALL}

all_noavahi:
	${MAKE}	STD=-DGDP_OSCF_USE_ZEROCONF=0 LIBAVAHI= OBJS_ZC= all

libgdpjs.a: ${OBJS}
	${AR} -r $@ ${OBJS}
	${RANLIB} $@

# Mac and Linux have different naming conventions, so we
# conditionalize a script instead of having rules.  If we ran
# configure somewhere, then we could avoid this, but configure has its
# own set of complexities.  This rule generates the libgdpjs shared
# library and then creates links in the libs directory that include
# the gdp and ep shared libraries.  We need to have all three
# libraries so that we can build a npm package.  Unfortunatley, RHEL
# and Ubuntu seem to have different versioning conventions, so we need
# both libxx-n.m.so and libxx.so.n-m.
#
# libcrypto and libssl are different under RHEL vs. Ubuntu, so
# we have different shared libraries for RHEL.
#
# There is no need to ship the libep shared library with the GDP/JS
# interface because libgdp and libgdpjs have linked with the libep
# static library.
libgdpjs.so.${GDPJSLIBVER}: ${OBJS}
	${CC} ${SHARED} -o $@ ${OBJS} ${LDFLAGS}
	@echo "gdpjs.js expects to load 'libgdp.${GDPLIBVER}' etc., so use that style versioning in libs/."
	if [ "`uname -s`" = "Darwin" ]; then \
		cp libgdpjs.so.${GDPJSLIBVER} ${LIBDIR}/libgdpjs.${GDPJSLIBVER}.dylib; \
		if [ -f ../../../libs/libgdp.${GDPLIBVER}.dylib ]; then \
			echo "Copying ../../../libs/libgdp.${GDPLIBVER}.dylib to ${LIBDIR}"; \
			cp ../../../libs/libgdp.${GDPLIBVER}.dylib ${LIBDIR}/; \
		fi; \
		if [ -f $(PTII)/lib/libgdpjs.${GDPJSLIBVER}-rhel.so ]; then \
			echo "Copying Linux shared libraries from $(PTII)"; \
			cp $(PTII)/lib/libgdpjs.${GDPJSLIBVER}-rhel.so ${LIBDIR}; \
			cp $(PTII)/lib/linux-x86-64-rhel/libgdp.${GDPLIBVER}.so ${LIBDIR}/libgdp.${GDPLIBVER}-rhel.so; \
		fi; \
		if [ -f $(PTII)/lib/libgdpjs.${GDPJSLIBVER}.so ]; then \
			cp $(PTII)/lib/libgdpjs.${GDPJSLIBVER}.so ${LIBDIR}; \
			cp $(PTII)/lib/libgdp.${GDPLIBVER}.so ${LIBDIR}; \
		fi; \
	else \
		if [ -f /etc/redhat-release ]; then \
			echo "Setting up libgdpjs for RHEL"; \
			cp libgdpjs.so.${GDPJSLIBVER} ${LIBDIR}/libgdpjs.${GDPJSLIBVER}-rhel.so; \
			echo "Setting up libgdp for RHEL"; \
			cp ../../../libs/${LIBGDP}.so ${LIBDIR}/libgdp.${GDPLIBVER}-rhel.so; \
			echo "Created links in ${LIBDIR}/"; \
			if [ -f $(PTII)/lib/libgdpjs.${GDPJSLIBVER}.so ]; then \
				echo "Copying Linux shared libraries from $(PTII) for non-RHEL"; \
				cp $(PTII)/lib/libgdpjs.${GDPJSLIBVER}.so ${LIBDIR}; \
				cp $(PTII)/lib/libgdp.${GDPLIBVER}.so ${LIBDIR}; \
			fi; \
		else \
			echo "Setting up libgdpjs"; \
			cp libgdpjs.so.${GDPJSLIBVER} ${LIBDIR}/libgdpjs.${GDPJSLIBVER}.so; \
			echo "Setting up libgdp"; \
			cp ../../../libs/libgdp*${LIBGDPVER}.so ${LIBDIR}/libgdp.${GDPLIBVER}.so; \
			echo "Created links in ${LIBDIR}/"; \
			if [ -f $(PTII)/lib/libgdpjs.${GDPJSLIBVER}-rhel.so ]; then \
				echo "Copying Linux shared libraries from $(PTII) for RHEL"; \
				cp $(PTII)/lib/libgdpjs.${GDPJSLIBVER}-rhel.so ${LIBDIR}; \
				cp $(PTII)/lib/linux-x86-64-rhel/libgdp.${GDPLIBVER}.so ${LIBDIR}/libgdp.${GDPLIBVER}-rhel.so; \
			fi; \
		fi; \
		if [ -f $(PTII)/lib/libgdpjs.${GDPJSLIBVER}.dylib ]; then \
			echo "Copying Darwin shared libraries from $(PTII)"; \
			cp $(PTII)/lib/libgdpjs.${GDPJSLIBVER}.dylib ${LIBDIR}; \
			cp $(PTII)/lib/libgdp.${GDPLIBVER}.dylib ${LIBDIR}; \
		fi; \
	fi \

gdpjs_version.o: gdpjs_version.c FORCE
	${CC} ${CFLAGS} -D_CURRENT_DATE_=\"`date +'%Y-%m-%d_%H:%M'`\" -c gdpjs_version.c

FORCE:


# When uploading a npm package, we want to include shared libraries
# for the other platform.

# Here, we copy them from $PTII.  A better solution would be to check
# in the shared library files somewhere.

# If we are running under Darwin, then copy the .so files from $PTII/lib.
# If we are running under Linux, then copy the .dylib files from $PTII/lib.

# Darwin shared libraries.
LIBGDPDYLIB=libgdp.${GDPLIBVER}.dylib
../libs/${LIBGDPDYLIB}:
	if [ ! -f ../libs/${LIBGDPDYLIB} ]; then \
		cp ${PTII}/lib/${LIBGDPDYLIB} $@; \
	fi

LIBGDPJSDYLIB=libgdpjs.${GDPJSLIBVER}.dylib
../libs/${LIBGDPJSDYLIB}:
	if [ ! -f ../libs/${LIBGDPSJDYLIB} ]; then \
		cp ${PTII}/lib/${LIBGDPJSDYLIB} $@; \
	fi


# Linux shared libraries
LIBGDPSO=libgdp.${GDPLIBVER}.so
../libs/${LIBGDPSO}:
	if [ ! -f ../libs/${LIBGDPSO} ]; then \
		cp ${PTII}/lib/${LIBGDPSO} $@; \
	fi

LIBGDPJSSO=libgdpjs.${GDPJSLIBVER}.so
../libs/${LIBGDPJSSO}:
	if [ ! -f ../ep/${LIBGDPSJSO} ]; then \
		cp ${PTII}/lib/${LIBGDPJSSO} $@; \
	fi

# All the shared libraries necessary  for npm.
NPMLIBS=../libs/${LIBGDPDYLIB} ../libs/${LIBGDPJSDYLIB} ../libs/${LIBGDPSO} ../libs/${LIBGDPJSSO}

# Copy the libraries for the other platform from $PTII.
copyNpmLibs: $(NPMLIBS)

npmPublish: $(NPMLIBS)
	(cd ..; npm publish -access public)


#
#  Administrative stuff
#

clean:
	-${RM} -f ${ALL} *.o *.core
	-${RM} -f ../libs/*

${OBJS}: ${HFILES}
