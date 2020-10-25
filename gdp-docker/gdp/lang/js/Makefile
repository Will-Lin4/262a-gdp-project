#
#  Makefile for gdp/lang/js/  (based on gdp/Makefile)
#
#	$Id$
#
#  ----- BEGIN LICENSE BLOCK -----
#	GDP: Global Data Plane
#	From the Ubiquitous Swarm Lab, 490 Cory Hall, U.C. Berkeley.
#
#	Copyright (c) 2014-2016, Regents of the University of California.
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
#  ----- END LICENSE BLOCK -----
#

# Alec Dara-Abrams
# 2014-11-04
#
#
# Caveats:
#
#    This Makefile assumes that it is located in gdp/lang/js/ ,
#    where gdp/ is a GDP Git local repository.
#
#    It will not force a re-build of the system up in gdp/ .
#    gdp/ must be re-built and kept up to date separately.
#
#    It invokes corresponding builds in sub-directories gdpjs/, apps/,
#    and tests/ .
#
#    Products of the build:
#      1) See sub-directories gdpjs/, apps/, and tests/ for their build products
#      2) a sub-directory, node_modules, is managed here.  It contains Node.js
#         packages needed by Node.js JS code in gdpjs/ apps/ and tests/ .
#         These packages are likely platform-specific and need to be installed
#         and build dynamically.


# External makefile targets below: all, clean, test


# Internal makefile variables

# Support for loading and building necessary Node.js JS modules
# npm - Node.js Package Manager, currently included in the node system
NPM  = npm
# directory for modules
NMOD = node_modules
# placeholder, not currently used
NODE = node
# Node modules necessary to run the JavaScript interface.
NODE_MODULES = $(NMOD)/ffi $(NMOD)/jsesc     $(NMOD)/posix-getopt \
               $(NMOD)/ref $(NMOD)/ref-array $(NMOD)/ref-struct \
               $(NMOD)/sleep


# External makefile targets: all, clean, test

all: $(NODE_MODULES) adm/gdp-setup.sh adm/common-support.sh
	-rm libs/*
	(cd gdpjs; make clean all)
	(cd apps;  make clean all)


all_noavahi: $(NODE_MODULES) adm/gdp-setup.sh adm/common-support.sh
	-rm libs/*
	(cd gdpjs; make clean all_noavahi)
	(cd apps;  make clean all)


# We need a copy of the setup scripts so that node @terraswarm/gdp
# users can install the necessary packages.

adm/gdp-setup.sh: adm ../../adm/gdp-setup.sh
	sed 's/\(package .*pandoc\)/#\1/' ../../adm/gdp-setup.sh > $@

adm/common-support.sh: adm ../../adm/common-support.sh
	cp ../../adm/common-support.sh $@

adm:
	if [ ! -d adm ]; then \
		mkdir adm; \
	fi

clean:
	-rm libs/*
	-rm -fr -- $(NMOD)
	(cd gdpjs; make clean)
	(cd apps;  make clean)

tests:
	mocha test/mocha

# TBD: Tag file builders exist for JavaScript - find one and include it.
tags: .FORCE
	ctags `find . -name '*.[ch]' -o -name '*.js' -o -name '*.cc'`

.FORCE:

# Internal makefile targets

$(NMOD)/ffi:
	$(NPM) install ffi
$(NMOD)/jsesc:
	$(NPM) install jsesc
$(NMOD)/posix-getopt:
	$(NPM) install posix-getopt
$(NMOD)/ref:
	$(NPM) install ref
$(NMOD)/ref-array:
	$(NPM) install ref-array
$(NMOD)/ref-struct:
	$(NPM) install ref-struct
$(NMOD)/sleep:
	$(NPM) install sleep
