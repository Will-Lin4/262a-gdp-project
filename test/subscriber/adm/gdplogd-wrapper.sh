#!/bin/sh
#
#  Wrapper for starting up gdplogd
#

# allow environment to give us a different configuration; local overrides
: ${GDP_VER=}
{ test -r /etc/gdp.conf.sh && . /etc/gdp.conf.sh; }
{ test -r /usr/local/etc/gdp.conf.sh && . /usr/local/etc/gdp.conf.sh; }
{ test -r /etc/gdp${GDP_VER}.conf.sh && . /etc/gdp${GDP_VER}.conf.sh; }
{ test -r /usr/local/etc/gdp${GDP_VER}.conf.sh && . /usr/local/etc/gdp${GDP_VER}.conf.sh; }

# configure defaults
: ${GDP_ROOT:=/usr}
: ${GDP_LOG_DIR:=/var/log/gdp}
: ${GDP_USER:=gdp}
: ${GDPLOGD_ARGS:="-D*=10"}
: ${GDPLOGD_BIN:=$GDP_ROOT/sbin/gdplogd$GDP_VER}
: ${GDPLOGD_LOG:=$GDP_LOG_DIR/gdplogd.log}
: ${LLOGGER:="llogger -s3"}

# manifest constants (see <sysexits.h>)
EX_USAGE=64
EX_UNAVAILABLE=69
EX_NOPERM=77
EX_CONFIG=78

# if we are running as root, start over as gdp
test `whoami` = "root" && exec sudo -E -u $GDP_USER $0 "$@"

debug=false
if [ "x$1" = "x-D" ]
then
	debug=true
	shift
fi

if ! $debug
then
	# make sure log file exists so we can append to it
	test -f $GDPLOGD_LOG || cp /dev/null $GDPLOGD_LOG
	fifo=/tmp/gdp-logger.$$
	cleanup() {
		rm $fifo
	}
	trap cleanup EXIT
	if ! mkfifo -m 600 $fifo
	then
		echo "[FATAL] Cannot create fifo $fifo"
		exit $EX_NOPERM
	fi

	${LLOGGER} -a $GDPLOGD_LOG < $fifo &

	# redirect remaining output to the log
	exec > $fifo 2>&1
fi

echo "[INFO] Running $0 $@ as `whoami`"
if [ `whoami` != $GDP_USER ]
then
	echo "[WARN] Should be running as $GDP_USER"
fi
if [ $# -ne 0 ]
then
	echo "[ERROR] Usage: $0 [-D]"
	exit $EX_USAGE
fi

echo "[INFO] Running $GDPLOGD_BIN $GDPLOGD_ARGS"
$GDPLOGD_BIN $GDPLOGD_ARGS
rc=$?
echo "[FATAL] $0: $GDPLOGD_BIN exited with status $rc"
exit $rc
