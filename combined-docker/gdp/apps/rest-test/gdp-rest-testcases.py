#!/usr/bin/env python
#
#	RESTful interface to GDP - Test Suite
#
#	----- BEGIN LICENSE BLOCK -----
#	Applications for the Global Data Plane
#	From the Ubiquitous Swarm Lab, 490 Cory Hall, U.C. Berkeley.
#
#	Copyright (c) 2017, Regents of the University of California.
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
#	----- END LICENSE BLOCK -----

import os
import sys
import requests
import json
import fcntl
import re
import socket
import subprocess

#
# sanity check invocations
#
testbed = socket.gethostname()
if testbed != "gdp-rest-01":
    print "host is {}".format(testbed)
    print "Error: this script is only safe to run on the RESTful server"
    sys.exit(1)

#
# Monitor the gdp-rest-v2.log for diagnostic detail
#
log_path = "/var/log/gdp/gdp-rest-v2.log"
log = open(log_path)
log.seek(0, os.SEEK_END)
log_fd = log.fileno()
flags = fcntl.fcntl(log_fd, fcntl.F_GETFL)
fcntl.fcntl(log_fd, fcntl.F_SETFL, flags | os.O_NONBLOCK)

# dev null to silence subprocess output
dn = open("/dev/null", "w")

test_auth = None
#
# Create a temporary test account on gdp-rest-01 (do not use sample strings):
#
# $cat /etc/lighttpd/.plainauth
# your_unique_testuser:your_unique_testpassword
# [other_permanent_user_accounts_left_untouched]
# $
#
# ...fill in test_auth to match the above, and uncomment it
#
# test_auth = ("your_unique_testuser", "your_unique_testpassword")

if test_auth == None:
    print "Error: modify gdp-rest-testcases.py to use a temporary test account"
    sys.exit(1)

json_header = { 'Content-type': 'application/json' }

# clean up test logs (defined to be gdp-rest-01 local logs), while
# being careful to only remove keys which go with test logs, as all
# other keys are for real logs located in the real GDP.
def clean_glogs_and_keys():
    output = subprocess.check_output([ "sudo", "-g", "gdp", "-u", "gdp",
                                       "/usr/bin/find",
                                       "/var/swarm/gdp/glogs/", 
                                       "-type", "f", "-name", "*.glog" ])

    lines = output.splitlines()
    for line in lines:
        # find <glog_id> in /var/swarm/gdp/glogs/<glog_id>.glog
        glog_id = line[25:-5]
        # check expectations, where file removal is involved
        if len(glog_id) == 43:
            print "Info: remove local GCL {}...".format(glog_id),
            subprocess.call([ "sudo", "-g", "gdp", "-u", "gdp",
                              "/usr/bin/find", "/var/swarm/gdp/glogs/", 
                              "-type", "f", "-name", glog_id + "*",
                              "-exec", "/bin/rm", "-f", "{}", ";"])
            print "done"
            print "Info: remove local key {}...".format(glog_id),
            subprocess.call([ "sudo", "-g", "gdp", "-u", "gdp",
                              "/usr/bin/find", "/etc/gdp/keys/", 
                              "-type", "f", "-name", glog_id + ".pem",
                              "-exec", "/bin/rm", "-f", "{}", ";"])
            print "done"
        else:
            print "Error: cleanup has unexpected glog_id: {}".format(glog_id)

def page_display(p):
    if p != None:
        print "STATUS:"
        print p.status_code
        print "HEADERS:"
        print p.headers
        print "CONTENT:"
        print p.content
    else:
        print "Error: no page"
#

def failed_page_display(p):
    print "FAILED"
    print "DIAGNOSTICS:"
    page_display(p)
    print "END DIAGNOSTICS"
#

def failed_log_and_page_display(p, l):
    print "FAILURE DIAGNOSTICS:"
    print "==== gdp-rest-v2.log:"
    print l.read(-1)
    print "==== end"

    print "==== response page:"
    page_display(p)
    print "==== end"
    print "END DIAGNOSTICS"
#
    
def test_write(tc, tid, gid, rn):
    json_body = {
        tid : rn,
        "gcl_id" : gid
    }
    page = requests.post("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl/" +
                         gid,
                         auth = test_auth,
                         timeout = 60,
                         headers = json_header,
                         data = json.dumps(json_body))
    print "{} WRITE {}".format(tc, rn),
    if page.status_code == 200:
        print ""
        return True
    else:
        failed_page_display(page)
        return False
#

def test_read(tc, tid, gid, rn):
    page = requests.get("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl/" +
                         gid + "?recno=" + rn,
                         auth = test_auth,
                         timeout = 60)
    pj = page.json()
    print "{} READ {}".format(tc, rn),
    passed = False
    if page.status_code == 200 and pj[tid] == rn:
        print ""
        return True
    else:
        failed_page_display(page)
        return False
#

def test_log(tc, tid, gcl_id):
    print "{} GCL {} write and read".format(tc, gcl_id)

    if (test_write(tc, tid, gcl_id, "1") and
        test_write(tc, tid, gcl_id, "2") and
        test_write(tc, tid, gcl_id, "3") and
        test_read(tc, tid, gcl_id, "3") and
        test_read(tc, tid, gcl_id, "2") and
        test_read(tc, tid, gcl_id, "1")):
        print "{} PASSED".format(tc)
    else:
        print "{} FAILED GCL write and read".format(tc)
#
    
#
# HTTP PUT TESTS
#

def test_put_01():
    log.seek(0, os.SEEK_END)
    test_case = "TEST PUT 01:"
    test_id = "test_put_01_log"
    print "{} HTTP PUT new log".format(test_case)


    json_body = {
        "external-name" : "edu.berkeley.eecs.gdp-rest.test_put_01"
    }

    page = requests.put("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60,
                        headers = json_header,
                        data = json.dumps(json_body))
    if page.status_code == 201:
        pj = page.json()
        gcl_name = pj["gcl_name"]
        # DEPRECATED gdplogd_name retrieval code block
        # gdplogd_name = pj["gdplogd_name"]
        # if gcl_name != None and gdplogd_name != None:
        #     print "{} PASSED \"{}\" \"{}\"".format(test_case,
        #     gcl_name, gdplogd_name)
        #     test_log(test_case, test_id, gcl_name)
        # else:
        #     print "{} FAILED page: \"{}\" \"{}\"".format(test_case,
        #                                                  gcl_name, gdplogd_name)
        #     failed_log_and_page_display(page, log)
        # END DEPRECATED gdplogd_name retrieval code block
        if gcl_name != None:
            print "{} PASSED \"{}\"".format(test_case, gcl_name)
            test_log(test_case, test_id, gcl_name)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, gcl_name)
            failed_log_and_page_display(page, log)
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#

def test_put_02():
    log.seek(0, os.SEEK_END)
    test_case = "TEST PUT 02:"
    print "{} HTTP PUT existing log (from TEST PUT 01)".format(test_case)
    
    json_body = {
        "external-name" : "edu.berkeley.eecs.gdp-rest.test_put_01"
    }

    page = requests.put("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60,
                        headers = json_header,
                        data = json.dumps(json_body))
    if page.status_code == 409:
        pj = page.json()
        detail = pj["detail"]
        if detail == "external-name already exists on gdplogd server":
            print "{} PASSED \"{}\"".format(test_case, detail)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, detail)
            failed_log_and_page_display(page, log)
            
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#
        
def test_put_03():
    log.seek(0, os.SEEK_END)
    test_case = "TEST PUT 03:"
    print "{} HTTP PUT with no external-name".format(test_case)
    
    json_body = {
        "NO-external-name" : "edu.berkeley.eecs.gdp-rest.test_put_03"
    }

    page = requests.put("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60,
                        headers = json_header,
                        data = json.dumps(json_body))
    if page.status_code == 400:
        pj = page.json()
        detail = pj["detail"]
        if detail == "mandatory external-name not found":
            print "{} PASSED \"{}\"".format(test_case, detail)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, detail)
            failed_log_and_page_display(page, log)
            
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#

def test_put_04():
    log.seek(0, os.SEEK_END)
    test_case = "TEST PUT 04:"
    print "{} HTTP PUT with no request body".format(test_case)
    
    page = requests.put("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60,
                        headers = json_header)
    if page.status_code == 400:
        pj = page.json()
        detail = pj["detail"]
        if detail == "request body not recognized json format":
            print "{} PASSED \"{}\"".format(test_case, detail)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, detail)
            failed_log_and_page_display(page, log)
            
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#

def test_put_05():
    log.seek(0, os.SEEK_END)
    test_case = "TEST PUT 05:"
    print "{} HTTP PUT with no request header or body".format(test_case)
    
    page = requests.put("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60)
    if page.status_code == 400:
        pj = page.json()
        detail = pj["detail"]
        if detail == "request body not recognized json format":
            print "{} PASSED \"{}\"".format(test_case, detail)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, detail)
            failed_log_and_page_display(page, log)
            
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#

def test_put_06():
    log.seek(0, os.SEEK_END)
    test_case = "TEST PUT 06:"
    test_id = "test_put_06_log"
    print "{} HTTP PUT new log, verify all valid options".format(test_case)

    json_body = {
        "external-name" : "edu.berkeley.eecs.gdp-rest.test_put_06",
        "-C" : "swarmlab@berkeley.edu",
        "-h" : "sha224",
        "-k" : "dsa",
        "-b" : "1024",
        "-c" : "ignored_for_dsa",
        "META" : [ "meta1=foo", "meta2=bar" ],
    }

    page = requests.put("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60,
                        headers = json_header,
                        data = json.dumps(json_body))
    if page.status_code == 201:
        pj = page.json()
        gcl_name = pj["gcl_name"]
        # DEPRECATED gdplogd_name retrieval code block
        # gdplogd_name = pj["gdplogd_name"]
        # if gcl_name != None and gdplogd_name != None:
        #     print "{} PASSED \"{}\" \"{}\"".format(test_case,
        #     gcl_name, gdplogd_name)
        #     test_log(test_case, test_id, gcl_name)
        # else:
        #     print "{} FAILED page: \"{}\" \"{}\"".format(test_case,
        #                                                  gcl_name, gdplogd_name)
        #     failed_log_and_page_display(page, log)
        # END DEPRECATED gdplogd_name retrieval code block
        if gcl_name != None:
            print "{} PASSED \"{}\"".format(test_case, gcl_name)
            test_log(test_case, test_id, gcl_name)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, gcl_name)
            failed_log_and_page_display(page, log)
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#

#
# server controlled options -e, -S, -K should not be accessible to clients
#

def test_put_07():
    log.seek(0, os.SEEK_END)
    test_case = "TEST PUT 07:"
    print "{} HTTP PUT create log, -e option is not permitted".format(test_case)
    
    json_body = {
        "external-name" : "edu.berkeley.eecs.gdp-rest.test_put_07",
        "-e" : "none"
    }

    page = requests.put("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60,
                        headers = json_header,
                        data = json.dumps(json_body))
    if page.status_code == 400:
        pj = page.json()
        detail = pj["detail"]
        if detail == "request contains unrecognized json objects":
            print "{} PASSED \"{}\"".format(test_case, detail)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, detail)
            failed_log_and_page_display(page, log)
            
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#

def test_put_08():
    log.seek(0, os.SEEK_END)
    test_case = "TEST PUT 09:"
    print "{} HTTP PUT create log, -S option is not permitted".format(test_case)
    
    json_body = {
        "external-name" : "edu.berkeley.eecs.gdp-rest.test_put_08",
        "-S" : None
    }

    page = requests.put("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60,
                        headers = json_header,
                        data = json.dumps(json_body))
    if page.status_code == 400:
        pj = page.json()
        detail = pj["detail"]
        if detail == "request contains unrecognized json objects":
            print "{} PASSED \"{}\"".format(test_case, detail)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, detail)
            failed_log_and_page_display(page, log)
            
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#

def test_put_09():
    log.seek(0, os.SEEK_END)
    test_case = "TEST PUT 09:"
    print "{} HTTP PUT create log, -K option is not permitted".format(test_case)
    
    json_body = {
        "external-name" : "edu.berkeley.eecs.gdp-rest.test_put_09",
        "-K" : "/etc/gdp/keys"
    }

    page = requests.put("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60,
                        headers = json_header,
                        data = json.dumps(json_body))
    if page.status_code == 400:
        pj = page.json()
        detail = pj["detail"]
        if detail == "request contains unrecognized json objects":
            print "{} PASSED \"{}\"".format(test_case, detail)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, detail)
            failed_log_and_page_display(page, log)
            
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#

#
# unsupported options -G, -q, -s, -w, -W should not be accessible to clients
#

def test_put_10():
    log.seek(0, os.SEEK_END)
    test_case = "TEST PUT 10:"
    print "{} HTTP PUT create log, -G option is not permitted".format(test_case)
    
    json_body = {
        "external-name" : "edu.berkeley.eecs.gdp-rest.test_put_10",
        "-G" : "127.0.0.1"
    }

    page = requests.put("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60,
                        headers = json_header,
                        data = json.dumps(json_body))
    if page.status_code == 400:
        pj = page.json()
        detail = pj["detail"]
        if detail == "request contains unrecognized json objects":
            print "{} PASSED \"{}\"".format(test_case, detail)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, detail)
            failed_log_and_page_display(page, log)
            
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#

def test_put_11():
    log.seek(0, os.SEEK_END)
    test_case = "TEST PUT 11:"
    print "{} HTTP PUT create log, -q option is not permitted".format(test_case)
    
    json_body = {
        "external-name" : "edu.berkeley.eecs.gdp-rest.test_put_11",
        "-q" : None
    }

    page = requests.put("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60,
                        headers = json_header,
                        data = json.dumps(json_body))
    if page.status_code == 400:
        pj = page.json()
        detail = pj["detail"]
        if detail == "request contains unrecognized json objects":
            print "{} PASSED \"{}\"".format(test_case, detail)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, detail)
            failed_log_and_page_display(page, log)
            
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#

def test_put_12():
    log.seek(0, os.SEEK_END)
    test_case = "TEST PUT 12:"
    print "{} HTTP PUT create log, -s option is not permitted".format(test_case)
    
    json_body = {
        "external-name" : "edu.berkeley.eecs.gdp-rest.test_put_12",
        "-s" : "edu.berkeley.eecs.service.creation"
    }

    page = requests.put("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60,
                        headers = json_header,
                        data = json.dumps(json_body))
    if page.status_code == 400:
        pj = page.json()
        detail = pj["detail"]
        if detail == "request contains unrecognized json objects":
            print "{} PASSED \"{}\"".format(test_case, detail)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, detail)
            failed_log_and_page_display(page, log)
            
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#

def test_put_13():
    log.seek(0, os.SEEK_END)
    test_case = "TEST PUT 13:"
    print "{} HTTP PUT create log, -w option is not permitted".format(test_case)
    
    json_body = {
        "external-name" : "edu.berkeley.eecs.gdp-rest.test_put_13",
        "-w" : None
    }

    page = requests.put("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60,
                        headers = json_header,
                        data = json.dumps(json_body))
    if page.status_code == 400:
        pj = page.json()
        detail = pj["detail"]
        if detail == "request contains unrecognized json objects":
            print "{} PASSED \"{}\"".format(test_case, detail)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, detail)
            failed_log_and_page_display(page, log)
            
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#

def test_put_14():
    log.seek(0, os.SEEK_END)
    test_case = "TEST PUT 14:"
    print "{} HTTP PUT create log, -W option is not permitted".format(test_case)
    
    json_body = {
        "external-name" : "edu.berkeley.eecs.gdp-rest.test_put_14",
        "-W" : "/etc/gdp/keys"
    }

    page = requests.put("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60,
                        headers = json_header,
                        data = json.dumps(json_body))
    if page.status_code == 400:
        pj = page.json()
        detail = pj["detail"]
        if detail == "request contains unrecognized json objects":
            print "{} PASSED \"{}\"".format(test_case, detail)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, detail)
            failed_log_and_page_display(page, log)
            
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#

def test_put_15():
    log.seek(0, os.SEEK_END)
    test_case = "TEST PUT 15:"
    print "{} HTTP PUT create log, META list is None(s)".format(test_case)
    
    json_body = {
        "external-name" : "edu.berkeley.eecs.gdp-rest.test_put_15",
        "META" : None
    }

    page = requests.put("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60,
                        headers = json_header,
                        data = json.dumps(json_body))
    if page.status_code == 400:
        pj = page.json()
        detail = pj["detail"]
        if detail == "request contains unrecognized json objects":
            print "{} PASSED \"{}\"".format(test_case, detail)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, detail)
            failed_log_and_page_display(page, log)
            
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#

def test_put_16():
    log.seek(0, os.SEEK_END)
    test_case = "TEST PUT 16:"
    print "{} HTTP PUT create log, META list element bad(s)".format(test_case)
    
    json_body = {
        "external-name" : "edu.berkeley.eecs.gdp-rest.test_put_16",
        "META" : [ "foo=1", "missing_equal", "bar=2" ],
    }

    page = requests.put("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60,
                        headers = json_header,
                        data = json.dumps(json_body))
    if page.status_code == 400:
        pj = page.json()
        detail = pj["detail"]
        if detail == "request contains unrecognized json objects":
            print "{} PASSED \"{}\"".format(test_case, detail)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, detail)
            failed_log_and_page_display(page, log)
            
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#

#
# HTTP POST TESTS
#

def test_post_01():
    log.seek(0, os.SEEK_END)
    test_case = "TEST POST 01:"
    test_id = "test_post_01_log"
    print "{} HTTP POST new log".format(test_case)


    json_body = {
        "external-name" : None
    }

    page = requests.post("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60,
                        headers = json_header,
                        data = json.dumps(json_body))
    if page.status_code == 201:
        pj = page.json()
        gcl_name = pj["gcl_name"]
        # DEPRECATED gdplogd_name retrieval code block
        # gdplogd_name = pj["gdplogd_name"]
        # if gcl_name != None and gdplogd_name != None:
        #     print "{} PASSED \"{}\" \"{}\"".format(test_case,
        #     gcl_name, gdplogd_name)
        #     test_log(test_case, test_id, gcl_name)
        # else:
        #     print "{} FAILED page: \"{}\" \"{}\"".format(test_case,
        #                                                  gcl_name, gdplogd_name)
        #     failed_log_and_page_display(page, log)
        # END DEPRECATED gdplogd_name retrieval code block
        if gcl_name != None:
            print "{} PASSED \"{}\"".format(test_case, gcl_name)
            test_log(test_case, test_id, gcl_name)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, gcl_name)
            failed_log_and_page_display(page, log)
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#

def test_post_02():
    log.seek(0, os.SEEK_END)
    test_case = "TEST POST 02:"
    print "{} HTTP POST with an external-name".format(test_case)
    
    json_body = {
        "external-name" : "edu.berkeley.eecs.gdp-rest.test_post_02"
    }

    page = requests.post("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60,
                        headers = json_header,
                        data = json.dumps(json_body))
    if page.status_code == 400:
        pj = page.json()
        detail = pj["detail"]
        if detail == "POST external-name must have null value":
            print "{} PASSED \"{}\"".format(test_case, detail)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, detail)
            failed_log_and_page_display(page, log)
            
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#

def test_post_03():
    log.seek(0, os.SEEK_END)
    test_case = "TEST POST 03:"
    print "{} HTTP POST with no request body".format(test_case)
    
    page = requests.post("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60,
                        headers = json_header)
    if page.status_code == 400:
        pj = page.json()
        detail = pj["detail"]
        if detail == "request body not recognized json format":
            print "{} PASSED \"{}\"".format(test_case, detail)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, detail)
            failed_log_and_page_display(page, log)
            
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#

def test_post_04():
    log.seek(0, os.SEEK_END)
    test_case = "TEST POST 04:"
    print "{} HTTP POST with no request header or body".format(test_case)
    
    page = requests.post("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60)

    if page.status_code == 400:
        pj = page.json()
        detail = pj["detail"]
        if detail == "request body not recognized json format":
            print "{} PASSED \"{}\"".format(test_case, detail)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, detail)
            failed_log_and_page_display(page, log)
            
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#

def test_post_05():
    log.seek(0, os.SEEK_END)
    test_case = "TEST POST 05:"
    test_id = "test_post_05_log"
    print "{} HTTP POST new log, verify all valid options".format(test_case)

    json_body = {
        "external-name" : None,
        "-C" : "swarmlab@berkeley.edu",
        "-h" : "sha224",
        "-k" : "dsa",
        "-b" : "1024",
        "-c" : "ignored_for_dsa",
        "META" : [ "meta1=foo", "meta2=bar" ],
    }

    page = requests.post("https://gdp-rest-01.eecs.berkeley.edu/gdp/v2/gcl",
                        auth = test_auth,
                        timeout = 60,
                        headers = json_header,
                        data = json.dumps(json_body))
    if page.status_code == 201:
        pj = page.json()
        gcl_name = pj["gcl_name"]
        # DEPRECATED gdplogd_name retrieval code block
        # gdplogd_name = pj["gdplogd_name"]
        # if gcl_name != None and gdplogd_name != None:
        #     print "{} PASSED \"{}\" \"{}\"".format(test_case,
        #     gcl_name, gdplogd_name)
        #     test_log(test_case, test_id, gcl_name)
        # else:
        #     print "{} FAILED page: \"{}\" \"{}\"".format(test_case,
        #                                                  gcl_name, gdplogd_name)
        #     failed_log_and_page_display(page, log)
        # END DEPRECATED gdplogd_name retrieval code block
        if gcl_name != None:
            print "{} PASSED \"{}\"".format(test_case, gcl_name)
            test_log(test_case, test_id, gcl_name)
        else:
            print "{} FAILED page: \"{}\"".format(test_case, gcl_name)
            failed_log_and_page_display(page, log)
    else:
        print "{} FAILED status code: {}".format(test_case, page.status_code)
        failed_log_and_page_display(page, log)
#


#
# RUN TESTS
#

# #
# # preclean gdp-rest-01 v2 testbed (aborted test runs)
# #
# print "Info: stopping gdplogd2.service (purge GCL cache from prior test runs)"
# subprocess.call("sudo systemctl stop gdplogd2.service", shell=True)
# print "Info: stop gdp-rest-v2.service (purge GCL cache from prior test runs)"
# subprocess.call("sudo systemctl stop gdp-rest-v2.service", shell=True)
# print "Info: clean local logs (and their keys, but not production keys)"
# clean_glogs_and_keys()

# #
# # start v2 testbed
# #
# print "Info: start gdplogd2.service (sudo systemctl)"
# if subprocess.call("sudo systemctl start gdplogd2.service", shell=True) != 0:
#     print "Error: sudo systemctl start gdplogd2.service failed"
#     sys.exit(1)
# #
# log.seek(0, os.SEEK_END)
# print "Info: start gdp-rest-v2.service (sudo systemctl)"
# if subprocess.call("sudo systemctl start gdp-rest-v2.service", shell=True) != 0:
#     print "Error: sudo systemctl start gdp-rest-v2.service failed"
#     sys.exit(1)
# #
# m = re.search(".*could not initialize SCGI port.*", log.read(-1))
# if m != None:
#     print "Warning: gdp-rest has not released the SCGI port yet, wait and retry"
#     sys.exit(1)
# #

#
# TEST run
#

# HTTP PUT test suite
test_put_01()
test_put_02()
test_put_03()
test_put_04()
test_put_05()
test_put_06()
test_put_07()
test_put_08()
test_put_09()
test_put_10()
test_put_11()
test_put_12()
test_put_13()
test_put_14()
test_put_15()
test_put_16()

# HTTP POST test suite
test_post_01()
test_post_02()
test_post_03()
test_post_04()
test_post_05()

# print "Info: stopping gdplogd2.service (sudo systemctl) ...",
# if subprocess.call("sudo systemctl stop gdplogd2.service", shell=True) != 0:
#     print "\nError: systemctl stop gdplogd2.service failed"
# else:
#     print "stopped"

# print "Info: stopping gdp-rest-v2.service (sudo systemctl) ...",
# if subprocess.call("sudo systemctl stop gdp-rest-v2.service", shell=True) != 0:
#     print "\nError: systemctl stop gdp-rest-v2.service failed"
# else:
#     print "stopped"

# clean_glogs_and_keys()

dn.close()
log.close()

sys.exit(0)
#
