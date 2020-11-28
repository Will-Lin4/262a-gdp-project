#
#  ----- BEGIN LICENSE BLOCK -----
#	GDP: Global Data Plane
#	From the Ubiquitous Swarm Lab, 490 Cory Hall, U.C. Berkeley.
#
#	Copyright (c) 2015-2018, Regents of the University of California.
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

GDP_SRC_ROOT=	../..
GDP_ADM=	${GDP_SRC_ROOT}/adm
GDP_SRC_ALL=	Makefile	\
		git-version.txt	\
		adm		\
		apps		\
		doc		\
		ep		\
		examples	\
		gdp		\
		gdplogd		\
		lang		\
		libs		\
		scgilib		\
		services	\
		util		\

GDP_VER=	latest
DOCKER_TAG=	`${GDP_ADM}/gdp-get-version.sh`
DOCKER=		docker
DOCKERFLAGS=

ALL=		gdp-src-base gdp-dev-c gdp-run-base gdplogd
CLEANALL=	${TMPTAR} `basename ${TMPTAR}`

all: docker-build

docker-build: ${ALL}

#
#  Build individual images for external use.
#	Intermediate images are built automagically.
#

# image containing source code and prerequisites
TMPTAR=		/var/tmp/gdp-src.tar
TAR=		tar
gdp-src-base:	${GDP_ADM}/gdp-version.sh .FORCE
	${DOCKER} build ${DOCKERFLAGS} \
		--target gdp-system-base .
	rm -f ${GDP_ADM}/docker/gdp-src.tar
	cd ${GDP_SRC_ROOT} && \
		make clean && \
		${TAR} cf ${TMPTAR} \
			--exclude \*.BAK \
			--exclude \*.dSYM \
			--exclude \*~ \
			${GDP_SRC_ALL}
	mv ${TMPTAR} `basename ${TMPTAR}`
	${DOCKER} build ${DOCKERFLAGS} \
		--target gdp-src-base \
		--build-arg DOCKER_TAG=${DOCKER_TAG} \
		--build-arg GDP_SRC_TAR=`basename ${TMPTAR}` \
		-t gdp-src-base:latest \
		-t gdp-src-base:${DOCKER_TAG} .

# image for application development in C
gdp-dev-c:	${GDP_ADM}/gdp-version.sh .FORCE
	${DOCKER} build ${DOCKERFLAGS} --target gdp-dev-c \
		-t gdp-dev-c:latest \
		-t gdp-dev-c:${DOCKER_TAG} .

# image for application development in Python
gdp-dev-python:	${GDP_ADM}/gdp-version.sh .FORCE
	${DOCKER} build ${DOCKERFLAGS} --target gdp-dev-python \
		-t gdp-dev-python:latest \
		-t gdp-dev-python:${DOCKER_TAG} .

# image for application execution (minimal OS, base for other images)
gdp-run-base:	${GDP_ADM}/gdp-version.sh .FORCE
	${DOCKER} build ${DOCKERFLAGS} --target gdp-run-base \
		-t gdp-run-base:latest \
		-t gdp-run-base:${DOCKER_TAG} .

# image for GDP log server
gdplogd:	${GDP_ADM}/gdp-version.sh .FORCE
	${DOCKER} build ${DOCKERFLAGS} --target gdplogd \
		-t gdplogd:latest \
		-t gdplogd:${DOCKER_TAG} .

# image for GDP router
gdp_router:	${GDP_ADM}/gdp-version.sh .FORCE
	${DOCKER} build ${DOCKERFLAGS} --target gdp_router \
		-t gdp_router:latest \
		-t gdp_router:${DOCKER_TAG} .

clean:
	-rm -f ${CLEANALL} *.o *.core

#
#  Push images to our registry
#	Should only push those images appropriate for external use.
#	Assumes the DOCKER_REG_USER is logged in (see docker login --help
#		and ~/.docker/config.json).
#
DOCKER_REG_HOST=	gdp.cs.berkeley.edu:5005
DOCKER_REG_USER=	gdp
DOCKER_REG=		${DOCKER_REG_HOST}/${DOCKER_REG_USER}
UBUNTU_VER=		16.04
ALPINE_VER=		latest

docker-pull:
	${DOCKER} pull ubuntu:${UBUNTU_VER}
	${DOCKER} pull alpine:${ALPINE_VER}

docker-push: ${ALL}
	for image in ${ALL}; do \
		echo "Pushing $${image}:${DOCKER_TAG}"; \
		${DOCKER} tag $${image}:${DOCKER_TAG} ${DOCKER_REG}/$${image}:${DOCKER_TAG}; \
		${DOCKER} push ${DOCKER_REG}/$${image}:${DOCKER_TAG}; \
	done

${GDP_ADM}/gdp-version.sh:
	cd ${GDP_SRC_ROOT}/gdp && make gdp_version.h

.FORCE:
