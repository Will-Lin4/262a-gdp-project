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


## XXX XXX XXX
## XXX Obsolete. Needs update to the latest API version (ver 2)
## XXX XXX XXX

"""
A simple program to demonstrate subscriptions to multiple logs
"""

import sys
sys.path.append("../")
import gdp
import time

def main(*args):

    obj_name_mapping = {}

    for name_str in args:

        # create a python object
        gcl_name = gdp.GDP_NAME(name_str)

        # Assume that the GCL already exists
        gcl_handle = gdp.GDP_GCL(gcl_name, gdp.GDP_MODE_RO)
        obj_name_mapping[gcl_handle] = name_str

        # this is the actual subscribe call
        gcl_handle.subscribe(0, 0, None)

    while True:

        # This blocks, until there is a new event
        event = gdp.GDP_GCL.get_next_event(None)
        datum = event["datum"]
        gcl_name = obj_name_mapping[event["gcl_handle"]]
        readable_time = time.ctime(datum["ts"]["tv_sec"] +
                                    (datum["ts"]["tv_nsec"]*1.0/10**9))
        print_str = ">>> gcl_name: %s\n>>> recno: %d, ts: %s\n%s" % (gcl_name,
                                datum["recno"], readable_time, datum["data"])
        print print_str                    

if __name__ == "__main__":

    if len(sys.argv) < 2:
        print "Usage: %s <gcl-name1> <gcl-name2> ..." % sys.argv[0]
        sys.exit(1)

    # Change this to point to a gdp_router
    gdp.gdp_init()
    main(*sys.argv[1:])
