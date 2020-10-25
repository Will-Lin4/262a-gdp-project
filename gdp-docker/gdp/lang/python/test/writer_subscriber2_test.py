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
Test for writer_subscriber that creates a log, appends and
subscribes and then creates another log

See README.txt for instructions.

To run this test by hand, use
  python writer_subscriber2_test.py foo

"""

import sys
sys.path.append("../")
import gdp

import platform
import random
import string

def test_answer(logName):
    main(logName)
    

def create_append_subscribe():    
    # http://stackoverflow.com/questions/2257441/random-string-generation-with-upper-case-letters-and-digits-in-python
    name_str = 'python.test.writer_subscriber2_test.' + ''.join(random.SystemRandom().choice(string.ascii_uppercase + string.digits) for _ in range(10))
    print "Name: " + name_str;
    gcl_name = gdp.GDP_NAME(name_str);

    print "Trying to create using " + platform.node();
    logd_name = gdp.GDP_NAME(platform.node());

    print "About to create " + name_str 
    try:
        gdp.GDP_GCL.create(gcl_name, logd_name, '');
    except :
        # If run with "python writer_subscriber2_test.py foo"
        # and the router and logd daemons are not running, then we end up here.
        # FIXME: Probably don't want to hardcode in the log name
        logd_name = gdp.GDP_NAME('edu.berkeley.eecs.gdp-01.gdplogd');
        gdp.GDP_GCL.create(gcl_name, logd_name, '');
    print "Created " + name_str

    print "Get the writer"
    gcl_handle_writer = gdp.GDP_GCL(gcl_name, gdp.GDP_MODE_AO);

    print "Get the subscriber"
    gcl_handle_subscriber = gdp.GDP_GCL(gcl_name, gdp.GDP_MODE_RO)

    print "Make the subscribe call"
    # This is the actual subscribe call.
    gcl_handle_subscriber.subscribe(0, 0, None)

    print "About to loop"
    count = 0
    outputList = []
    while count < 10:
        count += 1
        line = str(count)
        # Create a minimalist datum dictionary
        datum = {"data": line}

        print "About to append data"
        gcl_handle_writer.append(datum)           # Write this datum to the GCL
        print "Done appending data"

        timeout = {'tv_sec':1, 'tv_nsec':0, 'tv_accuracy':0.0}

        print "About to call get_next_event()"

        event = gcl_handle_subscriber.get_next_event(timeout)
        datum = event["datum"]
        handle = event["gcl_handle"]
        print datum
        outputList.append(datum['data'])

    expectedList = ['1', '2', '3', '4', '5', '6', '7', '8', '9', '10']
    if outputList != expectedList:
        raise ValueError(', '.join(map(str, outputList)) + " was not equal to " + ', '.join(map(str, expectedList)))
    print "OK"

# To run this by hand:
#  python writer_subscriber_test2.py
def main(name_str):
    create_append_subscribe();
    create_append_subscribe();


if __name__ == "__main__":
    if len(sys.argv) != 1:
        print "Usage: %s " % sys.argv[0]
        print "Note that this test takes no arguments. %s will be ignored." % sys.argv[0]

    # Change this to point to a gdp_router
    gdp.gdp_init()
    main(sys.argv[1]);
