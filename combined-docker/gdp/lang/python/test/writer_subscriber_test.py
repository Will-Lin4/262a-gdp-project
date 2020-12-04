#!/usr/bin/env python
# ----- BEGIN LICENSE BLOCK -----
#	GDP: Global Data Plane
#	From the Ubiquitous Swarm Lab, 490 Cory Hall, U.C. Berkeley.
#
#	Copyright (c) 2015-2019, Regents of the University of California.
#	All rights reserved.
#
#	Permission is hereby granted, without written agreement and without
#	license or royalty fees, to use, copy, modify, and distribute this
#	software and its documentation for any purpose, provided that the above
#	copyright notice and the following two paragraphs appear in all copies
#	of this software.
#
#	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
#	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
#	PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
#	EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
#	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#	FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION,
#	IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO
#	OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS,
#	OR MODIFICATIONS.
# ----- END LICENSE BLOCK -----


"""
Test for writer_subscriber.

See README.txt for instructions.
"""

import sys
sys.path.append("../")
import gdp


def test_answer(logName):
    main(logName)
    
# To run this by hand:
#  export newLog=gdp.runPythonTests.newLog.$RANDOM
#  ../../../apps/gcl-create -k none -s edu.berkeley.eecs.gdp-01.gdplogd $newLog
#  python writer_subscriber_test.py $newLog
def main(name_str):

    # Create a GDP_NAME object from a python string provided as argument.
    print "Name: " + name_str
    gcl_name = gdp.GDP_NAME(name_str)
    gcl_handle_writer = gdp.GDP_GCL(gcl_name, gdp.GDP_MODE_AO);

    gcl_handle_subscriber = gdp.GDP_GCL(gcl_name, gdp.GDP_MODE_RO)

    # this is the actual subscribe call
    gcl_handle_subscriber.subscribe(0, 0, None)

    count = 0
    while count < 10:
        count += 1
        line = str(count)
        # Create a minimalist datum dictionary
        datum = {"data": line}
        gcl_handle_writer.append(datum)           # Write this datum to the GCL

        timeout = {'tv_sec':1, 'tv_nsec':0, 'tv_accuracy':0.0}

        print "About to call get_next_event()"

        event = gcl_handle_subscriber.get_next_event(timeout)
        datum = event["datum"]
        handle = event["gcl_handle"]
        print datum


if __name__ == "__main__":

    if len(sys.argv) < 2:
        print "Usage: %s <gcl_name>" % sys.argv[0]
        sys.exit(1)

    # Change this to point to a gdp_router
    gdp.gdp_init()
    main(sys.argv[1]);
