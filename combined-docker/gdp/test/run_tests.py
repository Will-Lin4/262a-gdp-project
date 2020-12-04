#!/usr/bin/env python

# ----- BEGIN LICENSE BLOCK -----
#	GDP: Global Data Plane
#	From the Ubiquitous Swarm Lab, 490 Cory Hall, U.C. Berkeley.
#
#	Copyright (c) 2016, Regents of the University of California.
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
Invoke the executables in this directory using Python 
so that we can get the output in JUnit compatible xml.

See README.txt for details.
"""

import socket
import subprocess

# Parsing the logName command line argument is set up in conftest.py.

def test_t_fwd_append(logName):
    subprocess.check_call(["./t_fwd_append", "-D *=18", logName, socket.gethostname()])

# Create the x00 log for use in other tests.  If the log already
# exists, then gcl-create returns 1, which can be ignored.
def test_gcl_create():
    try:
        subprocess.check_call(["../apps/gcl-create", "-k", "none", "-s", socket.gethostname(), "x00"]);
    except  subprocess.CalledProcessError as e:
        returncode = e.returncode
        if returncode != 73:
            print returncode
            raise

# Uses logName, which has some data.
# FIXME: t_multmultiread fails if the log is empty.
def test_t_multimultiread(logName):
    subprocess.check_call(["./t_multimultiread", "-D gdp.proto=37", logName])

# Uses the x00 log, which is new at this point.
#def test_t_multimultiread_x00(logName):
#    subprocess.check_call(["./t_multimultiread", "-D gdp.proto=37", "x00"])

# Uses the x00 log.
def test_t_sub_and_append(logName):
    subprocess.check_call(["./t_sub_and_append"])
    
