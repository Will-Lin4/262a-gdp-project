#!/bin/sh

#
#  Initialize GDP server hosts
#
#	This script should not be needed for ordinary clients.
#
#	This does not actually install the gdplogd software.
#	It is a helper script for the "make install" target in
#	gdplogd/Makefile that sets up the surrounding 
#
#	This should be portable to all environments.  In particular,
#	it is not dependent on a particular version of Linux (or
#	for that matter, Linux at all).
#
#	XXX  This assumes the Berkeley-based routers.  If you have
#	XXX  your own router you'll need to modify $EP_PARAMS/gdp
#	XXX  after this completes.
#
#	XXX  It also assumes you are in the eecs.berkeley.edu domain.
#	XXX  If you aren't you'll need to modify $EP_PARAMS/gdplogd
#	XXX  after this completes.
#
#	This script does not install a router or router
#	startup scripts.  The router is a separate package
#	and must be installed separately.
#

## be sure we're running as root
test `whoami` = "root" || exec sudo $0 "$@"

# we assume this is in the adm directory
cd `dirname $0`/..
GDP_SRC_ROOT=`pwd`
TMP=/tmp
. adm/common-support.sh

args=`getopt ir $*`
if [ $? != 0 ]
then
	echo "Usage: $0 [-i]"
	exit 1
fi

eval set -- "$args"
do_install=true
install_rest=false
while true
do
	case "$1"
	in
	  -i)
	  	do_install=false
		;;

	  -r)
		install_rest=true
		;;

	  --)
		shift
		break
		;;
	esac
	shift
done

# setup GDP user, basic directories, etc.
. adm/gdp-bin-setup.sh

. adm/gdp-version.sh
: ${GDP_VER=$GDP_VERSION_MAJOR}
: ${GDPLOGD_LOG:=$GDP_LOG_DIR/gdplogd.log}
: ${GDPLOGD_BIN:=$GDP_ROOT/sbin/gdplogd$GDP_VER}
export GDP_VER

echo "Installing gdplogd:"
(cd $GDP_SRC_ROOT/gdplogd && make install)

echo "Making server-specific directories"
mkdir_gdp $GDP_LOG_DIR
mkdir_gdp $GDP_VAR
mkdir_gdp $GDPLOGD_DATADIR 0750
mkdir_gdp /var/swarm
mkdir_gdp /var/swarm/gdp
mkdir_gdp /var/swarm/gdp/glogs

mkfile_gdp $GDPLOGD_LOG
$install_rest && mkfile_gdp $GDP_REST_LOG

info "Creating $EP_PARAMS/gdplogd"
{
	echo "swarm.gdplogd.gdpname=edu.berkeley.eecs.$hostname.gdplogd.physical"
	echo "swarm.gdplogd.runasuser=gdp"
} > $TMP/gdplogd.params
if [ ! -f $EP_PARAMS/gdplogd ]
then
	cp $TMP/gdplogd.params $EP_PARAMS/gdplogd
	chown ${GDP_USER}:${GDP_GROUP} $EP_PARAMS/gdplogd
	cat $EP_PARAMS/gdplogd
elif cmp -s $TMP/gdplogd.params $EP_PARAMS/gdplogd
then
	rm $TMP/gdplogd.params
else
	warn "$EP_PARAMS/gdplogd already exists; check consistency" 1>&2
	diff -u $TMP/gdplogd.params $EP_PARAMS/gdplogd
fi

info "Installing utility programs"
cd $GDP_SRC_ROOT
(cd util && make install)

info "Installing gdplogd wrapper script"
install -o ${GDP_USER} adm/gdplogd-wrapper.sh $GDP_ROOT/sbin/gdplogd${GDP_VER}-wrapper.sh

if $install_rest
then
	info "Installing gdp-rest wrapper script"
	install -o ${GDP_USER} adm/gdp-rest-wrapper.sh $GDP_ROOT/sbin/gdp-rest${GDP_VER}-wrapper.sh
fi

if [ -d /etc/rsyslog.d ]
then
	info "Installing rsyslog configuration"
	sh adm/customize.sh adm/60-gdp.conf.template /etc/rsyslog.d
	chown ${GDP_USER}:${GDP_GROUP} /etc/rsyslog.d/60-gdp.conf
fi

if [ -d /etc/logrotate.d ]
then
	info "Installing logrotate configuration"
	cp adm/gdp-logrotate.conf /etc/logrotate.d/gdp
fi

if [ "$INITSYS" = "systemd" ]
then
	info "Installing and enabling systemd service files"
	info "gdplogd.service ..."
	adm/customize.sh adm/gdplogd.service.template $TMP
	cp $TMP/gdplogd.service /etc/systemd/system/gdplogd$GDP_VER.service
	rm $TMP/gdplogd.service
	if $install_rest
	then
		info "gdp-rest.service ..."
		adm/customize.sh adm/gdp-rest.service.template /etc/systemd/system
	fi
	systemctl daemon-reload
	systemctl enable gdplogd$GDP_VER
	if $install_rest
	then
		systemctl enable gdp-rest
		warn "Startup scripts for gdp-rest are installed, but you will"
		warn "need to configure a web server to use the SCGI interface."
		warn "See README-CAAPI.md for advice."
	fi
else
	warn "No system initialization configured"
fi
