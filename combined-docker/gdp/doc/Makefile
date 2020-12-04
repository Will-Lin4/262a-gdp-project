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

# files that should be made public (installed in /usr/local/share/doc)
# 	It would be nice to make .html versions of the .md files, but
# 	pandoc is not installed on the Jenkins machine.
PUBALL=

# files that are auto-generated (should be cleaned)
CLEANALL= \
	  README.html

ALL=		${CLEANALL} ${PUBALL}

DESTDIR=
LOCALROOT=	/usr
INSTALLROOT=	${DESTDIR}${LOCALROOT}
DOCROOT=	${INSTALLROOT}/share/doc/gdp
DOCDIR=		${DOCROOT}/`${GDP_ADM}/gdp-get-version.sh mm`
GDP_SRC_ROOT=	..
GDP_ADM=	${GDP_SRC_ROOT}/adm
GDP_DOC=	${GDP_SRC_ROOT}/doc

all: ${ALL}
	(cd admin &&		${MAKE} all)
	(cd developer &&	${MAKE} all)
	(cd internal &&		${MAKE} all)

clean:
	test -z "${CLEANALL}" || rm -f ${CLEANALL}
	(cd admin &&		${MAKE} clean)
	(cd developer &&	${MAKE} clean)
	(cd internal &&		${MAKE} clean)

${DOCDIR}:
	mkdir -p ${DOCDIR}

install: ${DOCDIR} ${ALL}
	# temporary check to clean out old cruft:
	-(cd ${DOCROOT} && test -e gdp-programmatic-api.html && rm -r *)
	# and now back to your regularly scheduled program...
	-(cd ${DOCDIR} && rm -r *)
	test -z "${PUBALL}" || cp ${PUBALL} ${DOCDIR}
	cp ${CLEANALL} ${DOCDIR}
	(cd admin &&		${MAKE} install)
	(cd developer &&	${MAKE} install)
	(cd internal &&		${MAKE} install)

UPDATE_LICENSE=	${GDP_ADM}/update-license.sh

update-license:
	${UPDATE_LICENSE} Makefile ${ALL}
	(cd admin &&		${MAKE} update-license)
	(cd developer &&	${MAKE} update-license)
	(cd internal &&		${MAKE} update-license)

PANDOC=		pandoc
PANFLAGS=	-s -c ${GDP_DOC}/gdpdoc.css --self-contained

.SUFFIXES: .md .html .pdf

.md.html .md.pdf:
	${PANDOC} ${PANFLAGS} -o $@ $<
