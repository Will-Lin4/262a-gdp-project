#!/bin/sh

# Create a debian package that includes:
# - gdplogd
# - shared libraries needed by gdplogd

PACKAGE="gdp-server"
INSTALL_TARGET="install-gdplogd"

pkgdir=`dirname $0`
topdir="`( cd $pkgdir/../../ && pwd )`"
scriptdir=$topdir/adm/deb-pkg

if ! which checkinstall > /dev/null 2>&1; then
	echo "I need root permission to install checkinstall"
	sudo apt install checkinstall
fi

# Get the version number, making sure it exists first
( cd $topdir/gdp && make ../adm/gdp-version.sh )
. $topdir/adm/gdp-version.sh
VER=$GDP_VERSION_MAJOR.$GDP_VERSION_MINOR.$GDP_VERSION_PATCH

# Setup the files that checkinstall needs. No way to specify
#   the location. Sad.
cp $scriptdir/server-description.txt description-pak
cp $scriptdir/common-preinstall.sh preinstall-pak
cat $scriptdir/common-postinstall.sh $scriptdir/server-postinstall.sh > \
	postinstall-pak
cp $scriptdir/server-preremove.sh preremove-pak
chmod +x preinstall-pak postinstall-pak preremove-pak

# Build package
fakeroot checkinstall -D --install=no --fstrans=yes -y \
	    --exclude="$topdir,/usr/lib" \
	    --pkgname=$PACKAGE \
	    --pkgversion=$VER \
	    --pkglicense="See /LICENSE" \
	    --pkggroup="misc" \
	    --maintainer="eric@cs.berkeley.edu, mor@eecs.berkeley.edu" \
	    --requires="libevent-pthreads-2.0-5,
			libssl1.0.0,
			libjansson4,
			libprotobuf-c1,
			avahi-daemon,
			libavahi-client3,
			rsyslog" \
	    --nodoc \
	    --strip=no \
	    --stripso=no \
	    --addso=yes \
	    --gzman=yes \
	    --deldoc=yes \
	    --deldesc=yes \
	    make -C $topdir $INSTALL_TARGET

# cleanup
rm -f description-pak postinstall-pak preremove-pak
