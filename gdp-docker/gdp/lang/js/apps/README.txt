// JavaScript GDP access application programs
//
// Alec Dara-Abrams
// 2014-11-04

// These JS programs should all be run in directory gdp/lang/js/apps/ .
// They access JS and C libraries in gdp/lang/js/gdpjs/ .

// Currently, some GDP API documentation is available at:
// https://docs.google.com/document/d/1MdJ47NEfUQdJlTyAXwotZp8aJbXchRIi3VgwOz4LWuU/edit?usp=sharing 


// ===================================================================
README.txt
This file.


// ===================================================================
reader-test.js

JS version of reader-test.c . It takes the same command line arguments.

Usage:
node apps/reader-test.js [-D dbgspec] [-f firstrec] [-G gdpd_addr] [-m]
				      [-n nrecs] [-s] <gcl_name>

Simple examples:

cd .......gdp/lang/js/apps/
# Start a gdp daemon in the background.  
../../../../gdpd/gdpd &
node ./reader-test.js my_dummy_gcl_name
# reader-test.js should list the contents of the gcl written in the
# writer-test.js example below.

node ./reader-test.js -f -1 my_dummy_gcl_name
# reader-test.js should list the most recently read item of the gcl written
# in the writer-test.js example below.


// ===================================================================
writer-test.js
JS version of writer-test.c . It takes the same command line arguments.

Usage:
node apps/writer-test.js [-a] [-D dbgspec] [-G gdpd_addr] [<gcl_name>]

Simple example:

cd .......gdp/lang/js/apps/
# Make sure a gdp daemon is running on your system.
../../../gdpd/gdpd &
node ./writer-test.js my_dummy_gcl_name
# Enter some lines of content like:
# line01<CR> line02<CR> line03<CR> line04<CR> ^D
#   Note the termination of input with control-D.


// ===================================================================
rw_supt_test.js

JS test for functions in gdpjs/rw_supt.js 

Usage: See program.


// ===================================================================
server_running_rw_supt_test.js

Node.js-based minimal HTTP server which runs rw_supt_test.js on the host
machine upon being sent an HTTP GET.

Usage: See program.


// ===================================================================
