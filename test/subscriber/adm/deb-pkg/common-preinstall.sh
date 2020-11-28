#!/bin/sh

set -e

if [ ! -e /etc/apt/sources.list.d/mariadb.list ]; then
	echo "I need a newer version of mariadb.  Run:"
	echo "   apt install curl"
	echo "   curl -sS https://downloads.mariadb.com/MariaDB/mariadb_repo_setup | bash"
	echo "... and then try installing again using dpkg -i."
	exit 1
fi 1>&2
