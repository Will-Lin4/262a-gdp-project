#!/bin/sh

#
#  Print a version number; useful from Makefiles.
#  (Shell scripts can just use the values in $adm/gdp-version.sh.)
#
#	Can specify a "what" parameter to tell what part of the version
#	number to print:
#	"major": print major version number only
#	"minor": print minor version number only
#	"patch": print patch level only
#	"m" or "1": print major number (same as "major")
#	"mm" or "2": print major.minor number
#	"mmp" or "3" print major.minor.patch (default)
#

adm=`dirname $0`

. $adm/gdp-version.sh

what=${1:-3}

case "$what" in
   "1"|"major"|"m")
	ver="$GDP_VERSION_MAJOR"
	;;
   "2"|"mm")
	ver="$GDP_VERSION_MAJOR.$GDP_VERSION_MINOR"
	;;
   "minor")
	ver="$GDP_VERSION_MINOR"
	;;
   "3"|"mmp")
	ver="$GDP_VERSION_MAJOR.$GDP_VERSION_MINOR.$GDP_VERSION_PATCH"
	;;
   "patch")
	ver="$GDP_VERSION_PATCH"
	;;
   *)
	echo 1>&2 "Usage: $0 [ what ]"
	exit 64
	;;
esac

echo $ver
