#!/usr/bin/env sh

#
#  Initialize a container for running gdplogd.
#	This is run when a container is starting up.
#	We only need this script to expand environment variables.
#
#  It's important to use exec so that "docker stop" signals gdplogd,
#  not the shell.
#

: ${GDP_VER:=2}
echo exec /usr/sbin/gdplogd${GDP_VER} \
	-F \
	-G "$GDP_ROUTER" \
	-N "$GDPLOGD_NAME" \
	$GDPLOGD_ARGS \
	"$@"
exec /usr/sbin/gdplogd${GDP_VER} \
	-F \
	-G "$GDP_ROUTER" \
	-N "$GDPLOGD_NAME" \
	$GDPLOGD_ARGS \
	"$@"
