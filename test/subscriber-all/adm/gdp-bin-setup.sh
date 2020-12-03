#!/bin/sh

#
#  Set up system for binary installs.
#
#	Create gdp user, needed directories, etc.
#	Does not set up for compilation; for that use adm/gdp-setup.sh.
#

# we assume this is in the adm directory
cd `dirname $0`/..
TMP=/tmp
. adm/common-support.sh
(cd gdp; make ../adm/gdp-version.sh)
. adm/gdp-version.sh

#
#  Berkeley-specific configuration.  Can be overridden.
#

# externally visible routing nodes (gdp-02 and -04 firewalled)
berkeley_routers=`sed -e 's/#.*//' << 'EOF'
	gdp-01.eecs.berkeley.edu
#	gdp-02.eecs.berkeley.edu
	gdp-03.eecs.berkeley.edu
#	gdp-04.eecs.berkeley.edu
EOF
`

# Human-Oriented Name to GDPname Directory server
berkeley_hongd=gdp-hongd.cs.berkeley.edu

# Log (Data Capsule) creation service (GDPname)
berkeley_creation_service=edu.berkeley.eecs.gdp.service.creation

#
#  End of Berkeley-specific values
#

## be sure we're running as root
test `whoami` = "root" || exec sudo $0 "$@"

#
#  Now the configuration we actually use
#

# numeric user id --- we pick one for consistency
: ${GDP_UID:=133}

# pick a router set.  Define GDP_ROUTERS to use private routers.
: ${GDP_ROUTERS:=$berkeley_routers}

# HONGD server
: ${GDP_HONGD:=$berkeley_hongd}

# DataCapsule creation service (GDPname)
: ${GDP_CREATION_SERVICE:=$berkeley_creation_service}

info "Preparing install into GDP_ROOT=${GDP_ROOT}"
info "                        GDP_ETC=${GDP_ETC}"
info "             GDP_USER:GDP_GROUP=${GDP_USER}:${GDP_GROUP}"
info "                        GDP_UID=${GDP_UID}"
info "                    GDP_ROUTERS=${GDP_ROUTERS}"
info "                      GDP_HONGD=${GDP_HONGD}"
info "           GDP_CREATION_SERVICE=${GDP_CREATION_SERVICE}"

## create "gdp" group
if ! grep -q "^${GDP_GROUP}:" /etc/group
then
	info "Creating group $GDP_GROUP"
	addgroup --system --gid $GDP_UID $GDP_GROUP
fi

## create "gdp" user
if ! grep -q "^${GDP_USER}:" /etc/passwd
then
	info "Creating user $GDP_USER"
	adduser --system --uid $GDP_UID --ingroup $GDP_GROUP $GDP_USER
else
	info "Using existing $GDP_USER"
fi

umask 0022

# make the root directory
mkdir_gdp $GDP_ROOT
cd $GDP_ROOT

## create system directories
if [ "$GDP_ROOT" != "/usr" ]
then
	mkdir_gdp bin
	mkdir_gdp sbin
	mkdir_gdp lib
fi

# convert /etc/ep_adm_params => /etc/gdp/params
mkdir_gdp $GDP_ETC
EP_PARAMS=$GDP_ETC/params
if [ "$GDP_ETC" = "/etc/gdp" -o				\
     "$GDP_ETC" = "/usr/local/etc/gdp" -o		\
     "$GDP_ETC" = "/opt/local/etc/gdp" ]
then
	if [ -d `dirname $GDP_ETC`/ep_adm_params -a ! -e $GDP_ETC/params ]
	then
		# relocate /etc/ep_adm_params to /etc/gdp/params
		mv `dirname $GDP_ETC`/ep_adm_params $GDP_ETC/params
	fi
	if [ ! -e `dirname $GDP_ETC`/ep_adm_params ]
	then
		# create legacy link if it doesn't already exist
		(cd `dirname $GDP_ETC`; ln -s gdp/params ep_adm_params)
	fi
fi
if [ ! -d $GDP_ETC/params ]
then
	mkdir_gdp $EP_PARAMS
	(cd `dirname $GDP_ETC`; ln -s gdp/params ep_adm_params)
fi

mkdir_gdp $GDP_KEYS_DIR 0750

## set up default runtime administrative parameters
hostname=`hostname`

# determine default router set
routers=`echo $GDP_ROUTERS |
		tr ' ' '\n' |
		grep -v $hostname |
		shuf |
		tr '\n' ';' |
		sed -e 's/; *$//' `
if echo $GDP_ROUTERS | grep -q $hostname
then
	routers="127.0.0.1; $routers"
fi

installparams() {
	if [ ! -f $EP_PARAMS/$1 ]
	then
		mkfile_gdp $EP_PARAMS/$1
		cp $TMP/$1.params $EP_PARAMS/$1
		cat $EP_PARAMS/$1
	elif cmp -s $TMP/$1.params $EP_PARAMS/$1
	then
		rm $TMP/$1.params
	else
		warn "$EP_PARAMS/$1 already exists; check consistency" 1>&2
		diff -u $TMP/$1.params $EP_PARAMS/$1
	fi
}

info "Creating $EP_PARAMS/gdp"
{
	echo "swarm.gdp.routers=$routers"
	echo "swarm.gdp.hongdb.host=${GDP_HONGD}"
	echo "#libep.time.accuracy=0.5"
	echo "#libep.thr.mutex.type=errorcheck"
	echo "libep.dbg.file=stdout"
} > $TMP/gdp.params
installparams gdp

info "Creating $EP_PARAMS/gdp-create"
{
	echo "swarm.gdp.creation-service.name=${GDP_CREATION_SERVICE}"
} > $TMP/gdp-create.params
installparams gdp-create
