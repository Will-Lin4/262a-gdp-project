#!/bin/sh
{ test -r /usr/local/etc/gdp.conf.sh && . /usr/local/etc/gdp.conf.sh; } ||
	{ test -r /etc/gdp.conf.sh && . /etc/gdp.conf.sh; }

#
#  Wrapper for starting up gdp-rest
#

# configure defaults
: ${GDP_ROOT:=/usr}
: ${GDP_LOG_DIR:=/var/log/gdp}
: ${GDP_USER:=gdp}
: ${GDP_REST_ARGS:="-D*=10"}
: ${GDP_REST_BIN:=$GDP_ROOT/sbin/gdp-rest}
: ${GDP_REST_LOG:=$GDP_LOG_DIR/gdp-rest.log}
: ${LLOGGER:="llogger -s3"}

# manifest constants (see <sysexits.h>)
EX_USAGE=64
EX_UNAVAILABLE=69
EX_TEMPFAIL=75
EX_NOPERM=77
EX_CONFIG=78

# if we are running as root, start over as gdp
test `whoami` = "root" && exec sudo -u $GDP_USER $0 "$@"

debug=false
if [ "x$1" = "x-D" ]
then
	debug=true
	shift
fi

if ! $debug
then
	# make sure log file exists so we can append to it
	test -f $GDP_REST_LOG || cp /dev/null $GDP_REST_LOG
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

	${LLOGGER} -a $GDP_REST_LOG < $fifo &

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

rc=$EX_TEMPFAIL
while [ $rc -eq $EX_TEMPFAIL ];
do
	echo "[INFO] Running $GDP_REST_BIN $GDP_REST_ARGS"
	$GDP_REST_BIN $GDP_REST_ARGS
	rc=$?
	if [ $rc -eq $EX_TEMPFAIL ];
	then
		echo "[WARN] $0: $GDP_REST_BIN DNS lookup failure, will retry"
		sleep 30
	fi
done

echo "[FATAL] $0: $GDP_REST_BIN exited with status $rc"
exit $rc
