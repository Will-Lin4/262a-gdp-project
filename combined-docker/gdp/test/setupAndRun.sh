#!/bin/bash
# Use bash so that we get $RANDOM
# Check for gdplogd and gdp_router first, then create directories,
# then run the script optionally named as an argument, then kill the daemons.

# To use this script, create a script that runs the local tests that takes
# a logName as an argument.

# Process arguments

runScript=""
if [ $# -ge 1 ]; then
    runScript=$1
    if [ ! -x $runScript ]; then
        echo "$0: $runScript is not present or not executable"
        exit 2
    fi
fi

# Exit if we use a previously unset variable.
set -u

# See if the gdplogd directory is present.
grandparentDirectory=`dirname $0`/../..
sourceDirectory=`cd $grandparentDirectory; pwd`

gdplogd=$sourceDirectory/gdp/gdplogd/gdplogd

if [ ! -x "$gdplogd" ]; then
    echo "$0: $gdplogd was either not found or is not executable.  Exiting."
    exit 2
fi

######
# If necessary, check out the gdp_router directory.

gdpRouterSource=$sourceDirectory/gdp_router

if [ ! -d "$gdpRouterSource" ]; then
   echo "#### $0: Checking out the gdp_router repo with ssh and create $gdpRouterSource."
   mkdir -p `dirname $gdpRouterSource`
   (cd `dirname $gdpRouterSource`; git clone repoman@repo.eecs.berkeley.edu:/projects/swarmlab/gdp_router.git)
   if [ ! -d "$gdpRouterSource" ]; then
       echo "#### $0: Checking out with ssh failed, so we try https and create $gdpRouterSource."
       (cd `dirname "$gdpRouterSource"`; git clone https://repo.eecs.berkeley.edu/git/projects/swarmlab/gdp_router.git)
   fi
else
    echo "#### $0: Running git pull in $gdpRouterSource"
    (cd "$gdpRouterSource"; git pull)
fi
    
if [ ! -d "${gdpRouterSource}" ]; then
    echo "$0: Could not create ${gdpRouterSource}. Exiting"
    echo "Try running: "
    echo "   mkdir -p $sourceDirectory"
    echo "   (cd ~/sourceDirectory; git clone https://repo.eecs.berkeley.edu/git/projects/swarmlab/gdp_router.git)"
    echo "and then rerun this script."
    exit 3
fi


######
# Create the ep_adm_params directory.
# When running, we set the EP_PARAM_PATH variable.

EP_PARAM_PATH=$sourceDirectory/ep_adm_params

if [ ! -d "$EP_PARAM_PATH" ]; then
   mkdir -p "$EP_PARAM_PATH"
fi

export EP_PARAM_PATH=`cd $EP_PARAM_PATH; pwd`

echo "swarm.gdp.routers=localhost" > "$EP_PARAM_PATH/gdp"

######
# The default directory is /var/swarm/gdp/gcls, which must be created
# by root, but owned by the user that runs the gdplogd process.
# Instead, we use a separate log directory

GCLS=$sourceDirectory/gcls

echo "#### $0: Removing $GCLS and then recreating it."
rm -rf $GCLS
mkdir -p "$GCLS"

echo "swarm.gdplogd.gcl.dir=$GCLS" > "$EP_PARAM_PATH/gdplogd"

echo "#### $0: Set up $EP_PARAM_PATH"
echo " To run with these settings from the command line, use:"
echo "    export EP_PARAM_PATH=$EP_PARAM_PATH"

echo "#### $0: Starting gdp_router"
# pkill -f matches against the full argument list.
# RHEL: Use quotes with -f
pkill -u $USER -f 'python ./src/gdp_router.py'
startingDirectory=`pwd`
echo "Command to start gdp_router (cd $gdpRouterSource; python ./src/gdp_router.py -l routerLog.txt) &"
cd "$gdpRouterSource"
python ./src/gdp_router.py -l routerLog.txt &
gdp_routerPid=$!

sleep 2

echo "#### $0: Starting gdplogd"
echo "Command to start gdplogd: $gdplogd -F -N `hostname` &"
cd "$startingDirectory"
pkill -u $USER gdplogd
$gdplogd -F -N `hostname` &
gdplogdPid=$!

trap "kill $gdplogdPid $gdp_routerPid; pkill -9 -u $USER $gdplogdPid" INT

sleep 2

cd "$startingDirectory"
logName=gdp.runTests.$RANDOM
echo "#### Creating log $logName"
echo "Command to create a log: $sourceDirectory/gdp/apps/gcl-create -k none -s `hostname` $logName"
$sourceDirectory/gdp/apps/gcl-create -k none -s `hostname` $logName
returnValue=$?
if [ $returnValue -eq 73 ]; then
    echo "$0: Error: Log $logName already existed?"
fi

if [ $# -ge 1 ]; then
    # Run all the tests!
    echo "$0: Invoking $@ $logName"
    $@ $logName
    overallReturnValue=$?
else
    echo "$0: called with no arguments, gdp daemons running:"
    ps auxgw | grep gdp
    echo "$0: Sleeping..."
    sleep 9999999
fi

echo "#### $0: Stopping gdplogd and gdp_router"
kill $gdplogdPid $gdp_routerPid
pkill -9 -u $USER $gdplogdPid

exit $overallReturnValue

