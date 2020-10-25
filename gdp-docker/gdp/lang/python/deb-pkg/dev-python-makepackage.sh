#!/bin/sh

# ----- BEGIN LICENSE BLOCK ----- 
#	GDP: Global Data Plane
#	From the Ubiquitous Swarm Lab, 490 Cory Hall, U.C. Berkeley.
#
#	Copyright (c) 2015-2019, Regents of the University of California.
#	All rights reserved.
#
#	Permission is hereby granted, without written agreement and without
#	license or royalty fees, to use, copy, modify, and distribute this
#	software and its documentation for any purpose, provided that the above
#	copyright notice and the following two paragraphs appear in all copies
#	of this software.
#
#	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
#	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
#	PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
#	EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
#	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#	FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION,
#	IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO
#	OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS,
#	OR MODIFICATIONS.
# ----- END LICENSE BLOCK ----- 


# Create a debian package. Some reasons this should be a deb-package:
# - Certain dependencies (python/ctypes/psutil/...)
# - Ideal if installed in default python path
# - depends on location of libgdp and libep etc


PACKAGE="gdp-python"
curdir=`dirname $0`
topdir="`( cd $curdir/../ && pwd )`"
srcdir="`( cd $curdir/../../../ && pwd)`"
versionfile=$srcdir/adm/gdp-version.sh

pydir="/usr/lib/python2.7/dist-packages/"
sharedir="/usr/share/doc/gdp/gdp-python"

# Get the version number
( cd $srcdir/gdp && make ../adm/gdp-version.sh )
. $versionfile
VER=$GDP_VERSION_MAJOR.$GDP_VERSION_MINOR.$GDP_VERSION_PATCH

tmpdir="/tmp/"$PACKAGE"_"$VER
rm -rf $tmpdir
mkdir $tmpdir
mkdir -p $tmpdir/$pydir/gdp
mkdir -p $tmpdir/$sharedir/


cp -a $topdir/gdp/*.py $tmpdir/$pydir/gdp/.
cp -a $topdir/gdp/LICENSE $tmpdir/$pydir/gdp/.
# fix library locations
sed -i "s/\"..\", \"..\", \"..\", \"libs\"/\"\/\", \"usr\", \"lib\"/g" $tmpdir/$pydir/gdp/MISC.py

# documentation and examples
cp $topdir/README $tmpdir/$sharedir/
cp $topdir/LICENSE $tmpdir/$sharedir/
cp -a $topdir/apps/*.py $tmpdir/$sharedir/
cp -a $topdir/apps/LICENSE $tmpdir/$sharedir/
sed -i "s/sys.path.append/# sys.path.append/g" $tmpdir/$sharedir/*.py

mkdir $tmpdir/DEBIAN
echo "Package: $PACKAGE
Version: $VER
Priority: optional
Architecture: all
Depends: python (<< 2.8), python (>= 2.7~), gdp-dev-c (>=$VER), libevent-dev (>=2.0~), python-psutil (>= 0.5~)
Maintainer: Nitesh Mor <mor@eecs.berkeley.edu>
Description: A python interface for libGDP
 Some sample programs are located at $sharedir/examples
 Enjoy!" >> $tmpdir/DEBIAN/control

dpkg-deb --build $tmpdir .

rm -rf $tmpdir
