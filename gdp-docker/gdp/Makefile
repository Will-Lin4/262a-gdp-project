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

# DESTDIR is just for staging.  LOCALROOT should be /usr or /usr/local.
DESTDIR=
LOCALROOT=	/usr
INSTALLROOT=	${DESTDIR}${LOCALROOT}
DOCROOT=	${INSTALLROOT}/share/doc/gdp
DOCDIR=		${DOCROOT}/`${ADM}/gdp-get-version.sh mm`
CLEANEXTRA=	gdp-client*.deb gdp-server*.deb gdp-python*.deb \
		README*.html \
		libs/*.a libs/*.so* libs/*.dylib \

all: all-nodoc

all-nodoc:
	(cd ep &&	${MAKE} all)
	(cd gdp &&	${MAKE} all)
	(cd gdplogd &&	${MAKE} all)
	(cd apps &&	${MAKE} all)
	(cd util &&	${MAKE} all)
	(cd examples &&	${MAKE} all)
	(cd scgilib &&	${MAKE} all)

all-withdoc: all-nodoc
	(cd doc &&	${MAKE} all)	# needs pandoc

all-clientonly:
	(cd ep &&	${MAKE} all)
	(cd gdp &&	${MAKE} all)
	(cd apps &&	${MAKE} all)

# Build without avahi, the zero-conf facility that
# can be tricky to compile under Mac OS X.
all_noavahi:
	(cd ep &&	${MAKE} all)
	(cd gdp &&	${MAKE} all_noavahi)
	(cd scgilib &&	${MAKE} all)
	(cd gdplogd &&	${MAKE} all_noavahi)
	(cd apps &&	${MAKE} all_noavahi)
	(cd util &&	${MAKE} all)
	(cd examples &&	${MAKE} all_noavahi)

clean:
	(cd doc &&	${MAKE} clean)
	(cd ep &&	${MAKE} clean)
	(cd gdp &&	${MAKE} clean)
	(cd scgilib &&	${MAKE} clean)
	(cd gdplogd &&	${MAKE} clean)
	(cd apps &&	${MAKE} clean)
	(cd util &&	${MAKE} clean)
	(cd examples &&	${MAKE} clean)
	rm -f ${CLEANEXTRA}

# Install libraries needed to run (but not compile) applications
# (FIXME: this installs static as well as shared libraries)
install-lib:
	(cd ep &&	${MAKE} install-lib DESTDIR=${DESTDIR} LOCALROOT=${LOCALROOT})
	(cd gdp &&	${MAKE} install-lib DESTDIR=${DESTDIR} LOCALROOT=${LOCALROOT})

install-include:
	(cd ep &&	${MAKE} install-include DESTDIR=${DESTDIR} LOCALROOT=${LOCALROOT})
	(cd gdp &&	${MAKE} install-include DESTDIR=${DESTDIR} LOCALROOT=${LOCALROOT})

# Install code used by GDP-based application developers (including old names).
# For the time being client installs include development files.
install-dev-c install-client: install-include install-lib
	(cd gdp &&	${MAKE} install DESTDIR=${DESTDIR} LOCALROOT=${LOCALROOT})
	(cd apps &&	${MAKE} install DESTDIR=${DESTDIR} LOCALROOT=${LOCALROOT})

# Install code used by the log server
install-gdplogd: install-lib
	(cd gdplogd &&	${MAKE} install DESTDIR=${DESTDIR} LOCALROOT=${LOCALROOT})
	(cd util &&	${MAKE} install DESTDIR=${DESTDIR} LOCALROOT=${LOCALROOT})

# Install all documentation
install-doc:
	(cd doc &&	${MAKE} install DESTDIR=${DESTDIR} LOCALROOT=${LOCALROOT})
	mkdir -p ${DOCDIR}
	cp -rp examples ${DOCDIR}

# Used only for language installs (???)
install-local:
	(cd ep &&	${MAKE} install-local DESTDIR=${DESTDIR} LOCALROOT=${LOCALROOT})
	(cd gdp &&	${MAKE} install-local DESTDIR=${DESTDIR} LOCALROOT=${LOCALROOT})

# Split it into sub-targets to mimic our distribution, also
#   used by the debian packaging scripts. So if you change it here,
#   make sure you don't break the packaging.
install: install-dev-c install-gdplogd install-doc

GDPROOT=	~gdp
GDPALL=		adm/start-* \
		adm/run-* \
		apps/gcl-create \
		apps/gdp-rest \
		apps/gdp-reader \
		apps/gdp-writer \
		gdplogd/gdplogd \

CSRCS=		ep/*.[ch] \
		gdp/*.[ch] \
		gdplogd/*.[ch] \
		scgilib/scgilib.[ch] \
		apps/*.[ch] \

CTAGS=		ctags

tags: .FORCE
	${CTAGS} ${CSRCS}

.FORCE:

# Install Python interface in system paths. Optional (including old name)
install-dev-python install-python: install-dev-c
	(cd lang/python &&	$(MAKE) install) 

# Build the Java interface to the GDP. Optional for the GDP per se.
all_Java:
	(cd lang/java &&	${MAKE} clean all)

install_Java:
	(cd lang/java &&	${MAKE} clean install)

clean_Java:
	(cd lang/java &&	${MAKE} clean)

# Build the Node.js/JavaScript GDP accessing apps and the Node.js/JS
# RESTful GDP interface.  Optional for the GDP per se.
all_JavaScript:
	(cd lang/js &&		${MAKE} clean all)

clean_JavaScript:
	(cd lang/js &&		${MAKE} clean)

# Build the debian-style package.  Must be done on the oldest system
# around because of dependencies.

debian-package: all
	adm/deb-pkg/client/package.sh
	adm/deb-pkg/server/package.sh
	lang/python/deb-pkg/package.sh


ADM=		adm
UPDATE_LICENSE=	${ADM}/update-license.sh
GET_VERSION_INFO=	${ADM}/get-version-info.sh

update-license:
	${UPDATE_LICENSE} Makefile *.[ch]
	(cd ep &&	 ${MAKE} update-license)
	(cd gdp &&	 ${MAKE} update-license)
	(cd gdplogd &&	 ${MAKE} update-license)
	(cd apps &&	 ${MAKE} update-license)
	(cd doc &&	 ${MAKE} update-license)
	(cd lang &&	 ${MAKE} update-license)
	(cd test &&	 ${MAKE} update-license)
	(cd examples &&	 ${MAKE} update-license)
	(cd util &&	 ${MAKE} update-license)

git-version.txt: .FORCE
	${GET_VERSION_INFO} > /dev/null

# Not made by default
READMES_HTML= \
	README.html \
	README-admin.html \
	README-CAAPI.html \
	README-compiling.html \
	README-deb.html \
	README-developers.html \

PANDOC=		pandoc
PANFLAGS=	-s -c doc/gdpdoc.css

.SUFFIXES: .md .html

.md.html:
	${PANDOC} ${PANFLAGS} -o $@ $<

README_html: ${READMES_HTML}
