#!/bin/sh

# configuration
export GDP_NAME_ROOT="swarmlab.gdp.test.pcari_1"
log1="log1"
log2="log2"

# find local binaries and support files
dir=`dirname $0`
cd $dir/..
gdp=`pwd`
. adm/common-support.sh
export PATH="$gdp/apps:$PATH"

# create test logs if needed
log-exists $log1 || gdp-create -e none $log1
log-exists $log2 || gdp-create -e none $log2

warn "May need to manually add these logs to HONGD:"
warn "    gdp-name-add $GDP_NAME_ROOT/$log1 <internal name from above>"
warn "    gdp-name-add $GDP_NAME_ROOT/$log2 <internal name from above>"

# now run the test itself
cd $gdp/test
make t_common_support.o t_pcari_1
./t_pcari_1 $* $log1 $log2
