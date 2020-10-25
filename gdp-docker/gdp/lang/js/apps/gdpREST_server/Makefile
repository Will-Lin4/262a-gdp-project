#
#  Makefile for gdp/lang/js/apps/gdpREST_server/  (based on gdp/Makefile)
#
#	$Id$
#
#	Copyright (c) 2014, <TBD>.  All rights reserved.
#

# Alec Dara-Abrams
# 2014-11-06
#
# TBD: Copyrights, one-button tests, README files
#
# Caveats:
#
#    This Makefile assumes that it is located in
#    gdp/lang/js/apps/gdpREST_server/ ,
#    where gdp/ is a GDP Git local repository.
#
#    It will not force a re-build of the system up in gdp/ .
#    gdp/ must be re-built and kept up to date separately.


# ==========================================================================================
# External makefile targets below: all, clean, run, debug
# TBD: provide a test/ sub-directory.


# ------------------------------------------------------------------------
# Internal makefile variables

# Support for loading and building necessary Node.js JS modules
# npm - Node.js Package Manager, currently included in the node system
NPM  = npm
# directory for modules
NMOD = node_modules

# the actual Node.js interpreter
NODE = node

# a debugger for Node.js - part of the node-inspector npm package
# See https://www.npmjs.org/package/node-inspector .
NODE-INSPECTOR  = node-inspector
# node-debug is node-inspector's executable to start debugging
NODE-DEBUG      = node-debug
# node-debug's default port is 8080 which we would rather use for our actual Node.js server,
# gdpREST_server.js .  So we use 8081 here.
NODE-DEBUG-PORT = 8081

# ==========================================================================================
# External makefile targets: all, clean, run, debug

all: install
	@echo 'Nothing to make here - only JavaScript programs.'
	@echo 'Just a node_modules directory to do an npm install into.'
	@echo 'We use package.json to drive the install.'

clean:
	-rm -fr -- $(NMOD)

# TBD: also start a gdpd instance
run:
	@echo 'Start the GDP Node.js/JavaScript REST server.'
	@echo 'See the README here as well as the js/apps/ README.'
	@echo 'for details on how to run the JS programs here.'
	$(NODE) ./gdpREST_server.js

# See the internal target install-debug for (manual) Node.js debugger installation
# TBD: also start a gdpd instance
debug:
	@echo 'Start the GDP Node.js/JavaScript REST server using $(NODE-DEBUG),'
	@echo 'a Node.js debug interface.'
	@echo 'If $(NODE-DEBUG) does not start on your default browser (prefer Chrome),'
	@echo 'start a Chrome browser at http://localhost:$(NODE-DEBUG-PORT)/debug?port=5858'
	$(NODE-DEBUG) -p $(NODE-DEBUG-PORT) ./gdpREST_server.js 

# This runs dummy Mocha tests - placeholder
test: FORCE
	@echo 'TBD: Run some simple tests.'
	mocha --reporter spec

FORCE:

# ------------------------------------------------------------------------
# Internal makefile targets

install: $(NMOD)

$(NMOD): package.json
	$(NPM) install

install-debug:
	@echo '$(NODE-INSPECTOR) will be installed globally by $(NPM) - usually in'
	@echo '/usr/local/lib/node-modules/ with binaries, like $(NODE-DEBUG),'
	@echo 'in /usr/local/bin/ .'
	$(NPM) install -g $(NODE-INSPECTOR)


# ==========================================================================================
