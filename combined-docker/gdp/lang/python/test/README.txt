To start up the gdp_router and gdplogd and then run the tests, use:

  ./run.sh

That script invokes the ../../../test/setupAndRun.sh script, which in turn
invokes ../../../test/_internalRunPythonTests.sh

See ../../../test/README.txt for details.

If the daemons are already running, then to create a log and run the tests, use:

  export newLog=gdp.runPythonTests.newLog.$RANDOM
  ../../../apps/gcl-create -k none -s ealmac23.local $newLog
  py.test --logName=$newLog

If the daemons are not running, try:
  export newLog=gdp.runPythonTests.newLog.$RANDOM
  ../../../apps/gcl-create -k none -s edu.berkeley.eecs.gdp-01.gdplogd $newLog
  py.test --logName=$newLog

To run just one test, use:
  py.test --logName=$newLog xxx.py
  

