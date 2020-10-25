#!/bin/sh

#
#  90-hongd-init.sh --- initialize a HONGD database inside a Docker container.
#	Shares the same script that would be run outside a container.
#

# need GDP_SRC_ROOT to find support files
: ${GDP_SRC_ROOT:-/gdp}
export GDP_SRC_ROOT

# create HONGD tables and users
. /gdp/adm/hongd-init.sh
