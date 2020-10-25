JavaScript Global Data Plane access application programs and support libraries
==============================================================================

The Global Data Plane (GDP) is a middleware that provides a
data-centric glue for swarm applications. The basic primitive is that
of a secure single-writer append-only log stored on potentially
untrusted distributed infrastructure. Logs natively support a
publish-subscribe mode, making GDP a communication tool. At the same
time, data in a GDP log can be stored for long-term in a
fault-tolerant manner, thus making it an ideal tool for data-storage.

For more information about GDP, see the following publications:
- [Toward a Global Data Infrastructure](http://ieeexplore.ieee.org/xpl/login.jsp?tp=&arnumber=7436637)
- [The Cloud is Not Enough: Saving IoT from the Cloud](https://www.terraswarm.org/pubs/518.html)

This package provides a JavaScript interface to the GDP.

Resources
---------
* [https://gdp.cs.berkeley.edu](https://gdp.cs.berkeley.edu)
* [GDP API documentation](https://docs.google.com/document/d/1MdJ47NEfUQdJlTyAXwotZp8aJbXchRIi3VgwOz4LWuU/edit?usp=sharing) 
* [Discussion of the GDP JS interface](http://www.terraswarm.org/swarmos/wiki/Main/GDPJavaScriptInterface)

See also apps/README.txt.

Installation
------------
The libs/ directory contains shared libraries for Darwin, RHEL and
Ubuntu.  RHEL and Ubuntu libraries are needed because of issues with
libgdp need libcrypto and libcrypto needing different versions of
libssl.

Note that the GDP shared library included in this module requires
other packages.  To install the other packages, run

   ./adm/gdp-setup.sh

A modern version of npm is required, where modern is not npm 1.3.6.

We create a local `node_modules/` directory to make it easier to 
describe the installation process.  In practice, `npm` will install
in the `node_modules/` directory that is above the current directory.

  mkdir node_modules

Install using npm:

  npm install @terraswarm/gdp

Ignore messages about "ENOENT: no such file or directory, open 'xxx/package.json'".
To remove them, see [npm-autoint](https://www.npmjs.com/package/npm-autoinit).

Try out a test:

  npm install mocha
  cd node_modules/@terraswarm/gdp
  ../../mocha/bin/mocha -t 4000 test/mocha

What the test does is create a new log, write a string to it and then
read back the string from the log.

Issues
------
* The functions that are exported are funky. 
** The read_gcl_records() and write_gcl_records() functions take too many arguments and open and close the log each time
* Subscription via a callback is not yet supported.

Files
-----
* Makefile: Recursive make for gdpjs/ and apps/.
  Currently, apps/ does not build anything.

* README.md: This file.  Also, see README's in our subdirectories.

* apps/: JavaScript standalone applications programs.  In particular, apps/writer-test.js
  and apps/reader-test.js - both hand translations of corresponding gdp/apps/ 
  C programs.  These should also provide good examples of access to GDP from
  Node.js JavaScript.  Run with Node.js.  See apps/README.txt.

* adm/: The libgdp shared library depends on libevent2 and other libraries.
  To install the necessary libraries, run adm/gdp-setup.sh.

* gdpjs/: JavaScript and C support libraries.  Has a local Makefile that does build things.

* libs/: Running make in gdpjs/ populates the libs/ directory.  If $PTII
  is set, then shared libraries for other platforms are copied from $PTII/lib.

* node_modules/: Node.js modules required by these JS programs and
  libraries.  Loaded into our source repository via "npm install
  <module_name>" .

* test: Mocha test scripts.  To run, do:
	npm install mocha
	mocha test/mocha  


Updating this module and using those changes with the Node Accessor Host
========================================================================

To make channges visible to the Node Accessor Host, one can either
upload the changes to npm, which requires updating the version number
in package.json or one can install the module locally.

To install the module locally:

         cd $PTII/org/terraswarm/accessor/accessors/web/hosts/node
         npm install $PTII/vendors/gdp/gdp/lang/js

$PTII/org/terraswarm/accessor/accessors/web/hosts/node/node_modules/@terraswarm/gdp/
should be created or updated.

After that, one may edit
$PTII/org/terraswarm/accessor/accessors/web/hosts/node/node_modules/@terraswarm/gdp/gdpjs/gdpjs.js,
but don't forget to fold the changes back in to
$PTII/vendors/gdp/gdp/lang/js/gdpjs/gdpjs.js

Testing using Ptolemy II
========================
If necessary, update $PTII/lib:

        cp libs/libgdpjs.1.0.dylib $PTII/lib
        svn commit -m "Updated to gdp0.8.0." $PTII/lib/libgdpjs.1.0.dylib

Then run the model using Node:

        (cd $PTII/org/terraswarm/accessor/accessors/web/gdp/test/auto; node ../../../hosts/node/nodeHostInvoke.js -timeout 6000 gdp/test/auto/GDPLogCreateAppendReadJS)

The Node Host Composite Accessor creates a log on edu.berkeley.eecs.gdp-01.gdplogd, appends to it and reads from it.


Updating the GDP Version Number
==============================
(This is for the use of maintainers of this node module.)

If the GDP version number in ../../gdp/gdp_version.h changes, the make the following changes:

1. gdpjs/Makefile: Update:
        # Version of the GDP Library, should match ../../../gdp/Makefile
	GDPLIBMAJVER=	0
	GDPLIBMINVER=	8

2. gdpjs/gdpjs.js: Update

   // libcrypto and libssl are different under RHEL vs. Ubuntu, so
   // we have different shared libraries for RHEL.
   var libgdpPath = '/libs/libgdp.0.8';
   var libgdpjsPath = '/../libs/libgdpjs.1.0';
   try {
       var fs = require('fs');
       fs.accessSync('/etc/redhat-release', fs.F_OK);
       libgdpPath = '/libs/libgdp.0.8-rhel';
       libgdpjsPath = '/../libs/libgdpjs.1.0-rhel';
   } catch (exception) {
       // Not under RHEL
   }

3. Update package.json:
        "version": "0.8.0",

4. Run make all_noavahi

Or, see accessors/web/gdp/adm/gdpUpdatePtII.sh in
the accessors repo.  See https://www.icyphy.org/accessors/svn.html 

Install the npm @terrswarm/gdp package on the npm server.
========================================================
(This is for the use of maintainers of this node module.)

We are using an account named
'[terraswarm](https://www.npmjs.com/~terraswarm)' on the npmjs
repository to manage the @terraswarm/gdp package.

To update the @terraswarm/gdp package on npmjs:

1.  Update libgdp and libep:
        (cd ../..; make all)
2.  Update libgdpjs:
        make all
3.  Update the patch number in package.json
4.  Login to npm
        npm login

        Username: terraswarm
        Password: See ~terra/.npmpass on terra
        Email: terraswarm-software@terraswarm.org 
5.  Publish:
        npm publish --access public


