#!/bin/sh

lib=$1
major=$2
minor=$3
dir=$4

# can override search for GDP source root node by setting GDP_SRC_ROOT.
if [ -z "${GDP_SRC_ROOT-}" ]
then
	gdp=`pwd`
	while [ ! -d $gdp/gdp/adm ]
	do
		gdp=`echo $gdp | sed -e 's,/[^/]*$,,'`
		if [ -z "$gdp" ]
		then
			echo "[FATAL] Need gdp/adm directory somewhere in directory tree"
			exit 1
		fi
	done
	GDP_SRC_ROOT=$gdp/gdp
fi
. $GDP_SRC_ROOT/adm/common-support.sh

info "Creating lib$lib links in $dir"
cd $dir

ver=$major.$minor
case "$OS" in
  "ubuntu" | "debian" | "freebsd" | "centos")
	rm -f lib$lib.so.$major lib$lib.so lib$lib.$ver.so
	ln -s lib$lib.so.$ver lib$lib.so.$major
	ln -s lib$lib.so.$major lib$lib.so
	ln -s lib$lib.so.$ver lib$lib.$ver.so
	;;

  "darwin")
	rm -f lib$lib.$ver.dylib lib$lib.dylib
	mv lib$lib.so.$ver lib$lib.$ver.dylib
	ln -s lib$lib.$ver.dylib lib$lib.dylib
	install_name_tool -id lib$lib.$ver.dylib lib$lib.$ver.dylib
	;;

  "redhat")
	rm -f lib$lib-$ver.so lib$lib.so.$major lib$lib.so
	mv lib$lib.so.$ver lib$lib-$ver.so
	ln -s lib$lib-$ver.so lib$lib.so.$major
	ln -s lib$lib.so.$major lib$lib.so
	;;
esac
