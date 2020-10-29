##
##  Master Dockerfile for GDP core.  Has multiple targets:
##	gdp-system-base		The basics needed to compile the GDP.
##				Essentially equivalent to `adm/gdp-setup.sh`.
##	gdp-src-base		`gdp-system-base` plus GDP source code.
##	gdp-dev-c		The environment in which you can compile
##				C-based C programs.
##	gdp-dev-python		The Python development environment.
##	gdp-run-base		A minimal environment including the various
##				shared libraries needed in which to run
##				GDP-based programs.
##	gdplogd-dev
##	gdplogd			An image that runs the GDP log daemon.
##
##  You can build intermediate targets using:
##	docker build --target <target> -t <imagename> .
##
##  For example, to create just the C development environment:
##	docker build --target gdp-dev-c -t gdp-dev-c .
##
##  Parameters can be set during build time, but they are usually
##  deferred until run time when the container starts.  They can
##  be used for special circumstances such as building custom images
##  for special customers.
##	GDP_ROUTER		The default router for clients (IP address).
##	GDP_HONGD_SERVER	The Human to GDP name directory (IP address).
##	GDP_CREATION_SERVICE	The default creation service name (GDPname).
##	GDPLOGD_NAME		The default name for gdplogd (GDPname).
##				Can be changed at run time using
##				`docker run -e GDPLOGD_NAME=xxx gdplogd`.
##


############################################################
#
#  The operating system base, including required packages.
#  This is a separate step solely to avoid having to reinstall all of
#  these just because the GDP source code has changed.
#

FROM ubuntu:16.04 AS gdp-system-base
RUN apt-get update && apt-get install -y curl sudo && \
	curl -sS https://downloads.mariadb.com/MariaDB/mariadb_repo_setup | sudo bash && \
	apt-get install -y \
		apt-utils \
		build-essential \
		libavahi-client-dev \
		libevent-dev \
		libevent-pthreads-2.0.5 \
		libjansson-dev \
		libmariadb3 \
		libmariadb-dev \
		libsqlite3-dev \
		libssl-dev \
		libsystemd-dev \
		protobuf-c-compiler \
		uuid-dev \
	&& apt-get clean


############################################################
#
#  The source code.  This isn't intended for distribution or for
#  end user use, but is just a build step for other containers.
#	Installs prerequisite packages.
#	Installs the source code (from git).
#	Compiles (but does not install) the code.
#	Creates basic parameter files.
#
#  This is used as the basis for customer development environments,
#  system installs (e.g. gdplogd), and other similar images.
#
#  See comments below for parameters that can be specified at build
#  time or imported at run time.  Most of these can (for now) be
#  left unchanged if you are joining the Berkeley-based GDP cluster.
#  If you want to create a private universe you'll probably have to
#  change several of them.
#

FROM gdp-system-base as gdp-src-base

#  Parameters:

# The default routers (IP address or DNS names with semicolon separators);
# if not set, Berkeley routers are used.  For a private cluster they
# _must_ be defined.
ARG GDP_ROUTER
ENV GDP_ROUTER=$GDP_ROUTER

# The name of the creation service.  Default is the Berkeley cluster.
# For a private cluster, an alternative name should be used.
ARG GDP_CREATION_SERVICE
ENV GDP_CREATION_SERVCE=$GDP_CREATION_SERVICE

# The IP name of the Human-Oriented Name to GDPname directory server.
# This is actually a MariaDB or MySQL server.
ARG GDP_HONGD_SERVER
ENV GDP_HONGD_SERVER=$GDP_HONGD_SERVER

# The version number ("tag") of the resulting image.
ARG DOCKER_TAG=latest

# The location of a tar file containing the source tree
ARG GDP_SRC_TAR

# The major version number of the GDP code.  Should match GDP_VERSION_MAJOR
# in gdp/Makefile.  Used for creating file names.  It is not well defined.
ENV GDP_VER=2

# And now for the real work.
COPY setup-src.sh gdp-src.tar /src/
WORKDIR /src
RUN /src/setup-src.sh


############################################################
#
#  The C-based development environment.
#	Compiles and installs the libraries (but no clients).
#	Removes most of the source code when done.
#  The intent is that this will be used as a base image for C-based
#	application development.
#

FROM gdp-src-base AS gdp-dev-c
RUN /src/gdp/adm/docker/setup-dev-c.sh


############################################################
#
#  The Python-based development environment.
#  The intent is that this will be used as a base image for Python-based
#	application development.
#

#FROM gdp-src-base AS gdp-dev-python
#
#COPY setup-dev-python.sh /src/
#RUN /src/setup-dev-python.sh && rm /src/setup-dev-python.sh


############################################################
#
#  The base instance for a minimal execution environment.
#  Only to be used as a base for other containers.
#	XXX  Should be based on alpine, but I couldn't figure out how
#	XXX  to easily install the libraries needed to make the code
#	XXX  work.  Static linking failed, and `apk` is inscrutable.
#	XXX  This should get fixed someday.
#

#FROM alpine:latest AS gdp-run-base
FROM gdp-dev-c AS gdp-run-base

# The choice of uid 133 is arbitrary, but it should match the
# uid in the host's /etc/passwd file since the data volume
# is mounted from the host into the container.
ARG GDP_UID=133
#RUN addgroup -g $GDP_UID gdp && adduser -S -u $GDP_UID -G gdp gdp

COPY setup-run-base.sh /
RUN env GDP_UID=$GDP_UID /setup-run-base.sh && rm /setup-run-base.sh
VOLUME /etc/gdp

# could be in setup-run-base.sh
RUN rm -rf /src/gdp


############################################################
#
#  A gdplogd instance.
#
#	The data files are on an external volume.
#	Each container needs a known and unique id.
#	It is possible to override the default router.
#
#	You have to run this with:
#		datadir=/var/swarm/gdp
#		export GDP_ROUTER="<router name>"
#		export GDPLOGD_NAME="<unique name>"
#		docker run \
#			-v /etc/gdp:/etc/gdp:ro \
#			-v $datadir:/var/swarm/gdp \
#			-e GDP_ROUTER \
#			-e GDPLOGD_NAME \
#			gdplogd
#
#	The spawn-gdplogd.sh script should set this up for you, starting
#	from the host system parameter files.
#

# first a large image with source code
FROM gdp-src-base AS gdplogd-dev
RUN cd /src/gdp && make install-gdplogd

# then a smaller image to actually execute
FROM gdp-run-base AS gdplogd
ARG GDPLOGD_NAME
ENV GDPLOGD_NAME=$GDPLOGD_NAME
ARG GDP_ROUTER=gdp-01.eecs.berkeley.edu
ENV GDP_ROUTER=$GDP_ROUTER
VOLUME /var/swarm/gdp/glogs

COPY start-gdplogd.sh /usr/sbin/
COPY --from=gdplogd-dev \
	/src/gdp/gdplogd/gdplogd2 \
		/usr/sbin/
COPY setup-gdplogd.sh /src
RUN /src/setup-gdplogd.sh && rm /src/setup-gdplogd.sh

USER gdp:gdp

ENTRYPOINT [ "/usr/sbin/start-gdplogd.sh" ]
CMD [ "-D2" ]
