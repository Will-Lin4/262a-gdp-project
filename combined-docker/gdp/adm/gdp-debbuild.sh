#!/bin/sh

#
#  This script assumes you have installed the following packages:
#	build-essential
#	devscripts
#	debhelper
#

# staging area for building the package
stage=$HOME/gdp-stage

# email address for maintainer
email=$USER@cs.berkeley.edu

# address of repository
repo=repoman@repo.eecs.berkeley.edu:projects/swarmlab/gdp.git

# root of source tree
root=UNKNOWN



Reset='[0m'    # Text Reset

# Regular             Bold                  Underline             High Intensity        BoldHigh Intens       Background            High Intensity Backgrounds
Bla='[0;30m';     BBla='[1;30m';    UBla='[4;30m';    IBla='[0;90m';    BIBla='[1;90m';   On_Bla='[40m';    On_IBla='[0;100m';
Red='[0;31m';     BRed='[1;31m';    URed='[4;31m';    IRed='[0;91m';    BIRed='[1;91m';   On_Red='[41m';    On_IRed='[0;101m';
Gre='[0;32m';     BGre='[1;32m';    UGre='[4;32m';    IGre='[0;92m';    BIGre='[1;92m';   On_Gre='[42m';    On_IGre='[0;102m';
Yel='[0;33m';     BYel='[1;33m';    UYel='[4;33m';    IYel='[0;93m';    BIYel='[1;93m';   On_Yel='[43m';    On_IYel='[0;103m';
Blu='[0;34m';     BBlu='[1;34m';    UBlu='[4;34m';    IBlu='[0;94m';    BIBlu='[1;94m';   On_Blu='[44m';    On_IBlu='[0;104m';
Pur='[0;35m';     BPur='[1;35m';    UPur='[4;35m';    IPur='[0;95m';    BIPur='[1;95m';   On_Pur='[45m';    On_IPur='[0;105m';
Cya='[0;36m';     BCya='[1;36m';    UCya='[4;36m';    ICya='[0;96m';    BICya='[1;96m';   On_Cya='[46m';    On_ICya='[0;106m';
Whi='[0;37m';     BWhi='[1;37m';    UWhi='[4;37m';    IWhi='[0;97m';    BIWhi='[1;97m';   On_Whi='[47m';    On_IWhi='[0;107m';

warning()
{
	echo "${Yel}${On_Bla}[W] $1${Reset}"
}

error()
{
	echo "${Cya}${On_bla}[E] $1${Reset}"
}

fatal()
{
	echo "${Red}${On_Whi}[F] $1${Reset}" >&2
	exit 1
}


args=`getopt e:r:v: $*`
if [ $? != 0 ]
then
	fatal "Usage: [-e email] -r srcroot -v version XXX"
fi
set -- $args

ver=UNKNOWN
while true
do
	case "$1" in
	   -e)
		email=$2
		shift 2;;
	   -r)
		root=$2
		shift 2;;
	   -v)
		ver=$2
		shift 2;;
	   --)
		shift
		break;;
	esac
done

echo email=$email
echo root=$root
echo stage=$stage
echo ver=$ver
if [ "$ver" = "UNKNOWN" ]
then
	fatal "-v (version) flag must be specified"
fi

if [ "$root" = "UNKNOWN" ]
then
	warning "using staging area as source root"

	[ -d $stage ] || mkdir $stage || fatal "cannot access $stage"
	root=$stage
	cd $stage
	[ -e gdp-$ver ] && rm -rf gdp-$ver
	git clone --depth 1 $repo gdp-$ver
	rm -rf gdp-$ver/.git*
else
	cd $root
	[ -d gdp ] || fatal "cannot find gdp source tree"
	[ -d gdp-$ver ] && rm gdp-$ver
	ln -s gdp gdp-$ver || fatal "cannot create gdp-$ver"
fi
tar czf gdp_$ver.orig.tar.gz --exclude '.git*' gdp-$ver
cd gdp-$ver
debuild -us -uc
