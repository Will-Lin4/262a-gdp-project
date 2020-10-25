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
PUBALL= \
		gdp-server-admin.md \
		gdp-v2-cluster-setup-from-source.md \

# files that are auto-generated (should be cleaned)
CLEANALL= \
		gdp-server-admin.html \
		gdp-v2-cluster-setup-from-source.html \

ALL=		${CLEANALL} ${PUBALL}

DESTDIR=
LOCALROOT=	/usr
INSTALLROOT=	${DESTDIR}${LOCALROOT}
GDP_SRC_ROOT=	../..
GDP_ADM=	${GDP_SRC_ROOT}/adm
GDP_DOC=	${GDP_SRC_ROOT}/doc
DOCROOT=	${INSTALLROOT}/share/doc/gdp
DOCDIR=		${DOCROOT}/`${GDP_ADM}/gdp-get-version.sh mm`/admin

all: ${ALL}

clean:
	test -z "${CLEANALL}" || rm -f ${CLEANALL}

${DOCDIR}:
	mkdir -p ${DOCDIR}

install: ${DOCDIR} ${PUBALL}
	cp ${PUBALL} ${DOCDIR}

UPDATE_LICENSE=	${GDP_ADM}/update-license.sh
update-license:
	${UPDATE_LICENSE} Makefile ${ALL}

PANDOC=		pandoc
PANFLAGS=	-s -c ${GDP_DOC}/gdpdoc.css --self-contained

.SUFFIXES: .md .html .pdf

.md.html .md.pdf:
	${PANDOC} ${PANFLAGS} -o $@ $<
