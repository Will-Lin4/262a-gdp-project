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

GDP_SRC_ROOT=		../..
GDP_ADM=		${GDP_SRC_ROOT}/adm
GDP_DOC=		${GDP_SRC_ROOT}/doc
GDP_MARIADB_VERSION=	10.3

ALL=			common-support.sh

DOCKER=			docker
DOCKERFLAGS=
DOCKER_TAG=		`${GDP_ADM}/gdp-get-version.sh`

all: docker-build

install: docker-install

# Install mariadb natively (not in a Docker container)
native-install:
	env GDP_MARIADB_VERSION="${GDP_MARIADB_VERSION}" sh hongd-init.sh

clean:
	rm ${ALL}

docker-install: /etc/systemd/system/gdp-hongd.service

/etc/systemd/system/gdp-hongd.service: gdp-hongd.service.template
	${GDP_ADM}/customize.sh gdp-hongd.service.template /etc/systemd/system

docker-build: common-support.sh
	docker build \
		--build-arg GDP_MARIADB_VERSION="${GDP_MARIADB_VERSION}" \
		-t gdp/hongd:${DOCKER_TAG} \
		-t gdp/hongd:latest \
		.

common-support.sh: ${GDP_ADM}/common-support.sh
	cp $< $@

#
#  Push images to our registry
#	Should only push those images appropriate for external use.
#	Assumes the DOCKER_REG_USER is logged in (see docker login --help
#		and ~/.docker/config.json).
#
DOCKER_REG_HOST=	gdp.cs.berkeley.edu:5005
DOCKER_REG_USER=	gdp
DOCKER_REG=		${DOCKER_REG_HOST}/${DOCKER_REG_USER}

docker-pull:
	${DOCKER} pull mariadb:${GDP_MARIADB_VERSION}

docker-push: ${ALL}
	for image in ${ALL}; do\
		echo "Pushing $${image}:${DOCKER_TAB}"; \
		${DOCKER} tag $${image}:${DOCKER_TAG} ${DOCKER_REG}/$${image}:${DOCKER_TAG}; \
		${DOCKER} push ${DOCKER_REG}/$${image}:${DOCKER_TAG}; \
	done

PANDOC=		pandoc
PANFLAGS=	-s -c ${GDP_DOC}/gdpdoc.css --self-contained

.SUFFIXES: .md .html .pdf

.md.html .md.pdf:
	${PANDOC} ${PANFLAGS} -o $@ $<
