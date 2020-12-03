#!/usr/bin/env sh

#
#  Set up gdplogd configuration and database
#	This is run when the image is built.
#

set -e

# database directory inside container --- this will be a mounted volume
: ${GDP_DATA_ROOT:=/var/swarm/gdp}
mkdir -p $GDP_DATA_ROOT/glogs
chown gdp:gdp $GDP_DATA_ROOT $GDP_DATA_ROOT/glogs
chmod 750 $GDP_DATA_ROOT $GDP_DATA_ROOT/glogs

# parameter files will be imported from host environment
