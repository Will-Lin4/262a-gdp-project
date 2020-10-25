#!/bin/sh

#
#  Run this from the root of the GDP source tree
#

export GDP_ROOT=${1:-$HOME/tmp/gdp}
export GDP_KEYS=$GDP_ROOT/KEYS
export GDP_ETC=$GDP_ROOT/etc
export GDP_LOG_DIR=$GDP_ROOT/log
export GDP_USER=$USER

make_dir() {
	test -d $1 || mkdir -p $1
}

make_dir $GDP_ROOT
make_dir $GDP_LOG_DIR
make_dir $GDP_KEYS

make_dir $GDP_ROOT/bin 
make_dir $GDP_ROOT/etc
make_dir $GDP_ROOT/include
make_dir $GDP_ROOT/lib
make_dir $GDP_ROOT/sbin 
make_dir $GDP_ROOT/share/man

export LOCALROOT=$GDP_ROOT

make O=
make install LOCALROOT=$LOCALROOT

export PATH=$GDP_ROOT/bin:$GDP_ROOT/sbin:$PATH
export PROMPT_TAG=@

echo "==== Now entering environment with GDP_ROOT=$GDP_ROOT"
exec $SHELL
