gdp/test contains tests for the Global Data Plane.

The simplest way to run the tests is to invoke:
  ./run.sh

The setup is a bit convoluted, so some explanation is in order.

The primary goal is to provide JUnit-compatible XML for use with Jenkins.

A secondary goal is to have as little duplicated testing framework
code as possible.

Thus, we have a series of sh and Python scripts.

gdp/test (this directory) contains the testing framework.

gdp/lang/python/test uses the testing framework in this directory.

In this directory (gdp/test), how the run.sh script works is that
run.sh invokes this command:

  ./setupAndRun.sh ./_internalRunPythonTests.sh run_tests.py $@

The setupAndRun.sh script starts up the gdp_router and the gdplogd
daemon, creates a log and then invokes the _internalRunPythonTest.sh
script.

The _internalRunPythonTest.sh script eventually invokes:
  py.test run_tests.py --logName=$logName

The key elements here are:

"run_test.py" is the name of the script that was passed in.  If no file
names are passed in, then py.test will look for files named test_*.py
and *_test.py that have test* methods. See
https://pytest.org/latest/goodpractices.html

"--logName=$logName" is a py.test command line argument, which
is configured in conftest.py.  The value of the logName is set by
the setupAndRun.sh script when the log is created.


In order to generate JUnit-compatible xml, we run:
  ./run.sh --junitxml=../reports/junit/gdpTest.xml || true


If the daemons are running, try:
  export logName=gdp.test.newLog.$RANDOM
  ../apps/gcl-create -k none -s `hostname` $logName
  py.test --logName=$logName run_tests.py


The `enter-gdp-environment.sh` script sets up a parallel GDP
environment (binaries, libraries, etc.).  The directory name is
specified on the command line, and defaults to `$HOME/tmp/gdp`.
It then starts a subshell with `$PATH` set to use these binaries.
It must be run from the root of the source tree.
