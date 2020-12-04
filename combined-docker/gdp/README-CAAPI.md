% Other Interfaces To The Global Dataplane

***This documentation is out of date as of 2018-12-07.***

There are some other interfaces to the GDP, generally referred to
as CAAPIs (Common Access APIs).  For the most part these are lightly
documented.

The RESTful Interface
=====================

The RESTful interface was originally implemented and deployed in
support of the Signposts Project, which was demonstrated at the
TerraSwarm Annual Meeting held in October 2017. In addition to the
Signposts, several other TerraSwarm demonstrations made use of the
RESTful GDP Gateway deployment.

The RESTful gateway provided access to the GDP v1 Network until June
2019, when the implementation was adapted and transitioned to the
recently deployed GDP v2 Network.

If you want to use the deployed RESTful GDP Gateway, please browse to
the <https://gdp-rest-01.eecs.berkeley.edu/> home page, which will
present hotlinks to the RESTful API (`doc/gdp-rest-interface.html`)
and to python client usage example(s). Be advised that gdp-rest-01
will not respond to non-SSL-protected (http://) traffic, and requires
basic client authentication when executing RESTful actions. Please
contact the Swarm Lab for inquires regarding basic client credentials.
In addition to programmatic access, you can do GETs from inside a
browser such as Firefox or Chrome, but not POSTs.  To use other
methods you'll have to use Chrome.  Install the "postman" extension to
enable sending of arbitrary methods such as POST and PUT.

If you want to set up your own RESTful GDP Gateway, please review the
gdp-rest-01 server configuration details as documented on the GDP
redmine wiki. The production deployment uses a lighttpd server running
on Ubuntu 16.04, integrated with an SCGI backend. The gdp-rest daemon
handles requests from SCGI, and leverages gdp-create for log creation.

The RESTful GDP Gateway is limited to a RESTful-compatible subset of
GDP capabilities, though it provides an easy and rapid way to begin
using the GDP. A Web Socket Interface (`gdp-ws') has been deployed on
gdp-rest-01 to support access to some non-RESTful GDP functionality
(e.g. log subscriptions) in web browser environments.

Python Key-Value Store
======================

Key-Value lookups can be done in Python using
`lang/python/apps/KVstore.py`.  This is a library package for
incorporation into larger Python programs.

<!-- vim: set ai sw=4 sts=4 ts=4 : -->

<!-- Use
	pandoc -s -o README-CAAPI.html README-CAAPI.md
to process this to HTML -->
