#!/bin/sh

# Create a debian package that includes:
# - GDP C library
# - EP C library
# - Documentation
# - GDP applications
# It does not include
# - include files
# - other files of interest to developers only

PACKAGE="gdp-client"
INSTALL_TARGET="install-client"

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
cp $scriptdir/client-description.txt description-pak
cp $scriptdir/common-preinstall.sh preinstall-pak
cp $scriptdir/common-postinstall.sh postinstall-pak
chmod +x preinstall-pak postinstall-pak

# Build package
#	All we want in this package are the libraries.
#	For binaries only, requires libevent-pthreads-2.0.5,
#		libprotobuf-c1, libavahi-client3.
#	For developers (those writing programs that use the GDP),
#		the others dependencies are also required.
fakeroot checkinstall -D --install=no --fstrans=yes -y \
	    --exclude="$topdir,/usr/include" \
	    --pkgname=$PACKAGE \
	    --pkgversion=$VER \
	    --pkglicense="See /LICENSE" \
	    --pkggroup="libs" \
	    --maintainer="eric@cs.berkeley.edu, mor@eecs.berkeley.edu" \
	    --requires="libevent-pthreads-2.0-5,
			libssl1.0.0,
			libprotobuf-c1,
			libavahi-client3,
			libmariadb3,
			avahi-daemon" \
	    --nodoc \
	    --strip=no \
	    --stripso=no \
	    --addso=yes \
	    --gzman=yes \
	    --deldoc=yes \
	    --deldesc=yes \
	    make -C $topdir $INSTALL_TARGET

# cleanup
rm -f description-pak postinstall-pak
