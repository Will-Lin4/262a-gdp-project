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
A REST interface for a key-value store. Returns JSON encoded data.

In the current form, only a single key-value store is supported per instance 
of this program. Whether this should be changed is debatable.

One big limitation of REST interface is that data type for keys and values 
must be strings, binary strings are ok though. Hence, if a client wishes to 
store something more complicated, some serialization mechanism needs to be 
used (something like cPickle).


Use as follows:

Step 1: Start serving REST requests:
./<this-program-name> [-w] [-p port] logname keyfile
    -w: enable writes
    -p: TCP port to listen on

Step 2: Use a web-client (browser, wget, cURL. etc) to access REST interface.

Following requests are supported:
- GET /         (probably will go away sometime soon)
  => Lists all known keys, returned as a JSON string
- GET /key=<keyname>&ts=<timestamp>
  <keyname> :   URL encoded name of the key. Note that a key could potentially
                be a binary string. In case of binary data for keyname, it 
                should be divided into 8-bit bytes and then perform URL encoding.
  <timestamp> : optional timestamp. A decimal representation of floating point
                seconds since epoch. If present, the last known value of the
                key before this time is returned.
  => Returns a JSON string containing a dictionary with 'key', 'value' and 'ts'.
- PUT /key, with 'value' as body.
  Assigns (or updates) the value 'value' to 'key'.
  => Returns a JSON string containing a dictionary with 'key', 'value' and 'ts'.

TODO: Make sure JSON is a good enough choice for returning binary data?

"""

from twisted.internet import reactor
from twisted.web.resource import Resource
from twisted.web.server import Site

from KVstore import KVstore
import urlparse
import argparse
import time
import json


class KVstoreResource(Resource):


    isLeaf = True
    def __init__(self, logroot, keyfile, iomode):
        Resource.__init__(self)
        self.kvstore = KVstore(logroot, keyfile, iomode)

    def __get_to_dict(self, key, ts):
        temp = self.kvstore.get(key, ts)
        if temp is not None:
            t = {'key': key, 'value': temp[1], 'ts': temp[0]}
        else:
            t = {'key': key, 'value': None, 'ts': None}
        return t


    def render_GET(self, request):
        print "Received request:", request.uri
        if request.uri == "/":
            t = self.kvstore.keys()
        else:
            req = urlparse.parse_qs(request.uri[1:])
            key = req['key'][0]
            ts = float(req.get('ts', [time.time()])[0])
            t = self.__get_to_dict(key, ts)
        return json.dumps(t) + "\n"


    def render_PUT(self, request):
        val =  request.content.read()
        print "Received request:", request.uri, val
        key = request.uri[1:]
        self.kvstore[key] = val
        t = self.__get_to_dict(key, time.time()+86400)
        return json.dumps(t) + "\n"


if __name__ == '__main__': 

    parser = argparse.ArgumentParser()
    parser.add_argument("-w", "--write", help="Enable writes", 
                        action="store_true")
    parser.add_argument("-p", "--port", type=int, default=8811,
                        help="TCP port to serve requests on")
    parser.add_argument("logname", nargs='+', help="Name of the log")
    parser.add_argument("keyfile", nargs='+', help="Signing key file")

    args = parser.parse_args()

    mode = KVstore.MODE_RW if args.write else KVstore.MODE_RO
    site = Site(KVstoreResource(args.logname[0], args.keyfile[0], mode))
    reactor.listenTCP(args.port, site)
    print "Starting REST interface on port", args.port
    reactor.run()

