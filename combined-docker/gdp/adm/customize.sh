#!/bin/sh

#
#  Customize scripts for various pathnames and other parameters.
#
#	Defaults are defined here.
#
#	Usage: adm/customize.sh source-file target-dir
#

if [ $# -lt 1 -o $# -gt 2 ]
then
	echo 1>&2 "Usage: $0 source-file [target-dir]"
	exit 1
fi

source_file=$1
target_dir=${2:-}

source_dir=`dirname $source_file`
source_root=`basename $source_file .template`
source_file="$source_dir/$source_root.template"
#echo source_file=$source_file, source_dir=$source_dir, source_root=$source_root, target_dir=$target_dir

# allow environment to give us a different configuration; local overrides
: ${GDP_VER=}
{ test -r /etc/gdp.conf.sh && . /etc/gdp.conf.sh; }
{ test -r /usr/local/etc/gdp.conf.sh && . /usr/local/etc/gdp.conf.sh; }
{ test -r /etc/gdp${GDP_VER}.conf.sh && . /etc/gdp${GDP_VER}.conf.sh; }
{ test -r /usr/local/etc/gdp${GDP_VER}.conf.sh && . /usr/local/etc/gdp${GDP_VER}.conf.sh; }

# default configuration
: ${GDP_ROOT:=/usr}
if [ "$GDP_ROOT" = "/usr" ]
then
	: ${GDP_ETC:=/etc/gdp}
else
	: ${GDP_ETC:=$GDP_ROOT/etc}
fi
: ${GDP_LOG_DIR:=/var/log/gdp}
: ${GDP_SYSLOG_FACILITY:=local4}
: ${GDP_SYSLOG_LEVEL:=notice}
: ${GDP_USER:=gdp}
: ${GDP_VAR:=/var/swarm/gdp}
: ${GDP_VAR_RUN:=/var/run}
: ${GDP_KEYS_DIR:=$GDP_ETC/keys}
: ${GDPLOGD_BIN:=$GDP_ROOT/sbin/gdplogd$GDP_VER}
: ${GDPLOGD_ARGS:=}
: ${GDPLOGD_DATADIR:=}
: ${GDPLOGD_PIDFILE:=$GDP_VAR_RUN/gdplogd.pid}
: ${GDP_ROUTER_CONF:=$GDP_VAR_RUN/gdp-router-click.conf}
: ${SCGI_DEBUG:=1}
: ${SCGI_PORT:=8001}

if [ -z "$target_dir" ]
then
	target=/dev/stdout
else
	target=$target_dir/$source_root
fi

(
	echo "# Generated" `date +"%F %T %z"` from $source_file
	sed \
		-e "s;@GDP_VER@;$GDP_VER;g" \
		-e "s;@GDP_ROOT@;$GDP_ROOT;g" \
		-e "s;@GDP_ETC@;$GDP_ETC;g" \
		-e "s;@GDP_LOG_DIR@;$GDP_LOG_DIR;g" \
		-e "s;@GDP_SYSLOG_FACILITY@;$GDP_SYSLOG_FACILITY;g" \
		-e "s;@GDP_SYSLOG_LEVEL@;$GDP_SYSLOG_LEVEL;g" \
		-e "s;@GDP_USER@;$GDP_USER;g" \
		-e "s;@GDP_VAR@;$GDP_VAR;g" \
		-e "s;@GDP_VAR_RUN@;$GDP_VAR_RUN;g" \
		-e "s;@GDP_KEYS_DIR@;$GDP_KEYS_DIR;g" \
		-e "s;@GDPLOGD_BIN@;$GDPLOGD_BIN;g" \
		-e "s;@GDPLOGD_ARGS@;$GDPLOGD_ARGS;g" \
		-e "s;@GDPLOGD_DATADIR@;$GDPLOGD_DATADIR;g" \
		-e "s;@GDPLOGD_PIDFILE@;$GDPLOGD_PIDFILE;g" \
		-e "s;@GDP_ROUTER_CONF@;$GDP_ROUTER_CONF;g" \
		-e "s;@SCGI_DEBUG@;$SCGI_DEBUG;g" \
		-e "s;@SCGI_PORT@;$SCGI_PORT;g" \

	echo "# End of generated text" $1
) < $source_file > $target
