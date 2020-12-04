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
A simple program that performs the following using async ops
- writes a number of random messages, each of size fixed bytes.
- reads all the messages back
- verifies that what was written is exactly what is read back

"""


import sys
sys.path.append("../")
                # So that we can actually load the python_api module

import gdp    # load the main package
import random
import string


def generate_random_data(N, count):
    "returns an count-sized array of N-sized alphanumeric strings"

    ret = []
    charset = string.ascii_letters + string.digits
    for idx in xrange(count):
        ret.append(''.join(random.choice(charset) for _ in range(N)))

    return ret


def main(name_str, keyfile=None):

    # the data that will be written
    data = generate_random_data(1000, 1000)

    # XXX handling secret key at Python level doesn't work at the moment
    open_info = {}
    if keyfile is not None:
        skey = gdp.EP_CRYPTO_KEY(filename=keyfile,
                                keyform=gdp.EP_CRYPTO_KEYFORM_PEM,
                                flags=gdp.EP_CRYPTO_F_SECRET)
        open_info = {'skey': skey}

    gin_name = gdp.GDP_NAME(name_str)
    _name = "".join(["%0.2x" % ord(x) for x in gin_name.internal_name()])
    print "opening", _name
    gin_handle = gdp.GDP_GIN(gin_name, gdp.GDP_MODE_RA, open_info)

    # writing the actual data
    for (idx, s) in enumerate(data):
        print "writing message", idx
        datum = gdp.GDP_DATUM()
        datum["buf"].write(s)
        # C implementation does not handle more than
        # a single datum appends at a time... yet.
        gin_handle.append_async([datum])

    ## collect as many events as we had append operations
    for idx in xrange(len(data)):
        e = gin_handle.get_next_event(None)
        assert e["type"] == gdp.GDP_EVENT_CREATED or \
                e["type"] == gdp.GDP_EVENT_SUCCESS

    ################################
    ####  reading the data back ####
    ################################

    read_data = []
    gin_handle.read_by_recno_async(-1*len(data), len(data))
    while True:
        e = gin_handle.get_next_event(None)
        if e["type"] == gdp.GDP_EVENT_DONE:
            break
        assert e["type"] == gdp.GDP_EVENT_DATA
        read_data.append(e["datum"]["buf"].peek())

    print len(data), len(read_data)
    for idx in xrange(len(data)):               # verify correctness
        if data[idx] == read_data[idx]:
            print "message %d matches" % idx

if __name__ == "__main__":

    if len(sys.argv) < 2:
        print "Usage: %s <name> [<signing-key-file>]" % sys.argv[0]
        sys.exit(1)

    name_str = sys.argv[1]
    if len(sys.argv)>=3:
        keyfile = sys.argv[2]
    else:
        keyfile = None

    # Change this to point to a gdp_router
    gdp.gdp_init()
    main(name_str, keyfile)   # create a GCL with the given name
