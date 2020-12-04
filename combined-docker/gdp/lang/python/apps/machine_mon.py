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
An example program that logs some system information periodically.
This is just an example program that can be used as a starting point.

** Requires python-psutil to be installed **
"""

import sys
sys.path.append("../")
import gdp
import psutil
import time
import os
import socket
import json

SLEEP_INTERVAL = 60

##### global variables #####

# disk
last_read_count = 0
last_write_count = 0
last_read_bytes = 0
last_write_bytes = 0

# network
last_packets_sent = 0
last_packets_recv = 0
last_bytes_sent = 0
last_bytes_recv = 0


def getInfo():
    """ Returns a bunch of information as a dictionary """

    global last_read_count, last_write_count, \
            last_read_bytes, last_write_bytes

    global last_packets_sent, last_packets_recv, \
            last_bytes_sent, last_bytes_recv

    # Collect information that we are going to log
    d = {}
    d['time'] = time.time()
    d['host'] = socket.gethostname()

    # Using psutil categorization of cpu, memory, disks and network
    d['cpu_percent'] = psutil.cpu_percent(interval=None)
    d['memory_percent'] = psutil.virtual_memory().percent

    # disk
    iostat = psutil.disk_io_counters()
    try:
        assert last_read_count !=0 and last_write_count !=0 and \
                last_read_bytes != 0 and last_write_bytes !=0

        d['read_count'] = iostat.read_count - last_read_count
        d['write_count'] = iostat.write_count - last_write_count
        d['read_bytes'] = iostat.read_bytes - last_read_bytes
        d['write_bytes'] = iostat.write_bytes - last_write_bytes

        # make sure the counters aren't wrapped around
        assert d['read_count']>=0 and d['write_count']>=0 and \
                    d['read_bytes']>=0 and d['write_bytes']>=0

    except AssertionError:
        d['read_count'], d['write_count'] = 0, 0
        d['read_bytes'], d['write_bytes'] = 0, 0

    last_read_count = iostat.read_count
    last_write_count = iostat.write_count
    last_read_bytes = iostat.read_bytes
    last_write_bytes = iostat.write_bytes

    # network
    netstat = psutil.net_io_counters()
    try:
        assert last_packets_sent !=0 and last_packets_recv !=0 and \
                last_bytes_sent != 0 and last_bytes_recv !=0

        d['packets_sent'] = netstat.packets_sent - last_packets_sent
        d['packets_recv'] = netstat.packets_recv - last_packets_recv
        d['bytes_sent'] = netstat.bytes_sent - last_bytes_sent
        d['bytes_recv'] = netstat.bytes_recv - last_bytes_recv

        # make sure the counters aren't wrapped around
        assert d['packets_sent']>=0 and d['packets_recv']>=0 and \
                    d['bytes_sent']>=0 and d['bytes_recv']>=0

    except AssertionError:
        d['packets_sent'], d['packets_recv'] = 0, 0
        d['bytes_sent'], d['bytes_recv'] = 0, 0

    last_packets_sent = netstat.packets_sent
    last_packets_recv = netstat.packets_recv
    last_bytes_sent = netstat.bytes_sent
    last_bytes_recv = netstat.bytes_recv

    return d


def main(name_str, keyfile):

    skey = gdp.EP_CRYPTO_KEY(filename=keyfile,                                  
                                keyform=gdp.EP_CRYPTO_KEYFORM_PEM,              
                                flags=gdp.EP_CRYPTO_F_SECRET)                   

    # Create a GDP_NAME object from a python string provided as argument
    gcl_name = gdp.GDP_NAME(name_str)

    # There's a GCL with the given name, so let's open it
    gcl_handle = gdp.GDP_GCL(gcl_name, gdp.GDP_MODE_AO,
                                open_info={'skey':skey})

    while True:

        d = getInfo()
        # convert that to a nice string
        string_to_write = json.dumps(d)

        # Create a minimalist datum dictionary
        datum = {"data": string_to_write}
        gcl_handle.append(datum)           # Write this datum to the GCL
        time.sleep(SLEEP_INTERVAL)


if __name__ == "__main__":

    if len(sys.argv) < 3:
        print "Usage: %s <gcl_name> <signing-keyfile>" % sys.argv[0]
        sys.exit(1)

    # Change this to point to a gdp_router
    gdp.gdp_init()
    main(sys.argv[1], sys.argv[2])
