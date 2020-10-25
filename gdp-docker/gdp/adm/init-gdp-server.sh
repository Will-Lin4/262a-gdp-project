#!/bin/sh

#
#  Initialize GDP server hosts
#
#	This script should not be needed for ordinary clients.
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

args=`getopt i $*`
if [ $? != 0 ]
then
	echo "Usage: $0 [-i]"
	exit 1
fi

eval set -- "$args"
moreargs=true
do_install=true
while $moreargs
do
	case "$1"
	in
	  -i)
		do_install=false;
		;;

	  --)
		moreargs=false
		shift
		break
		;;
	esac
	shift
done

# we assume this is in the adm directory
cd `dirname $0`/..
GDP_SRC_ROOT=`pwd`
TMP=/tmp
. adm/common-support.sh

## compile code and utilities, possibly not as root
info "Compiling GDP code and utilities"
make
(cd util && make)

## be sure we're running as root
test `whoami` = "root" || exec sudo GDP_REST_INSTALL=$GDP_REST_INSTALL $0 "$@"

# setup GDP user, basic directories, etc.
. adm/gdp-bin-setup.sh

. adm/gdp-version.sh
: ${GDP_VER=$GDP_VERSION_MAJOR}
: ${GDPLOGD_LOG:=$GDP_LOG_DIR/gdplogd.log}
: ${GDPLOGD_BIN:=$GDP_ROOT/sbin/gdplogd$GDP_VER}
: ${GDP_REST_INSTALL:=false}
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
mkfile_gdp $GDP_REST_LOG

info "Creating $EP_PARAMS/gdplogd"
{
	echo "swarm.gdplogd.gdpname=edu.berkeley.eecs.$hostname.gdplogd"
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
install -o ${GDP_USER} adm/gdplogd-wrapper.sh \
	$GDP_ROOT/sbin/gdplogd${GDP_VER}-wrapper.sh

if $GDP_REST_INSTALL
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
	if $GDP_REST_INSTALL
	then
		info "gdp-rest.service ..."
		adm/customize.sh adm/gdp-rest.service.template /etc/systemd/system
	fi
	systemctl daemon-reload
	systemctl enable gdplogd$GDP_VER
	if $GDP_REST_INSTALL
	then
		systemctl enable gdp-rest
		warn "Startup scripts for gdp-rest are installed, but you will"
		warn "need to configure a web server to use the SCGI interface."
		warn "See README-CAAPI.md for advice."
	fi
else
	warn "No system initialization configured"
fi
