#!/bin/bash
# Run the Python tests.
# Usage: runPythonTests.sh logName
#
# An alternative way is to run gdp/test/setupAndRun.sh, which will start the daemons:
#   ../../../test/setupAndRun.sh ./runPythonTests.sh

if [ $# -lt 1 ]; then
    echo "$0: Usage: $0 logName [py.test args]"
fi
# Get the last argument
logName="${@: -1}"

# Run the tests

failedTests=""
overallReturnValue=0

function runTest () {
    echo "#### $@"
    $@
    returnValue=$?
    if [ $returnValue != 0 ]; then
        failedTests="$failedTests $1"
        overallReturnValue=$returnValue
    fi
}

echo "Command: py.test ${@:1:$(($#-1))} --logName=$logName"
runTest py.test -v ${@:1:$(($#-1))} --logName=$logName

#newLog=gdp.runPythonTests.newLog.$RANDOM
#../../../apps/gcl-create -k none -s ealmac23.local $newLog
#runTest ./KVstore.py $newLog

if [ $overallReturnValue != 0 ]; then
    exit $overallReturnValue
fi

# gdp/test/setupAndRun.sh will stop the daemons
 
