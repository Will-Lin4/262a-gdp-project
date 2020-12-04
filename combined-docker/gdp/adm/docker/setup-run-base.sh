#!/usr/bin/env sh

#
#  Script to set up the run environment for GDP-based apps.
#  This runs inside the gdp-run-base container.
#

set -e

args=`getopt D $*`
test $? != 0 && echo "Usage $0" >&2 && exit 64
set -- $args
debug=false
while true
do
	$debug && echo "Parsing $*"
	case "$1" in
	  -D)
		debug=true
		;;
	  --)
		shift
		break;;
	esac
	shift
done

# GDP_UID should match that of the enclosing system
adduser --system --uid ${GDP_UID:=133} --group gdp

# /etc/gdp will be a volume, usually mounted from /etc/gdp
: ${GDP_ETC:=/etc/gdp}
mkdir -p $GDP_ETC $GDP_ETC/params
chown gdp:gdp $GDP_ETC $GDP_ETC/params
chmod 775 $GDP_ETC $GDP_ETC/params

# back compatability
cd /etc
ln -s gdp/params ep_adm_params
