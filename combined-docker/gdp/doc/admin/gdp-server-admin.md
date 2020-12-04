% Administering Global Dataplane Servers

These instructions are relevant *only* if you are running a GDP router
and/or log daemon.  In most cases you should be able to use the
servers at Berkeley.

This assumes that the server code is already installed and ready to
run.  If you have not used a package, see `README-server-install.md`
in the root of the source tree for instructions on installing from
source.

Considerations for the GDP Data
===============================

For obvious reasons GDP data files should be on a filesystem
that has a reasonable amount of free space.  The default is
`/var/swarm/gdp/glogs`.  The `adm/init-gdp-server.sh` script
creates this directory if it doesn not already exist, but if
you want to put it on a separate filesystem you might want to
do that in advance.  For example:

	sudo mkdir -p /var/swarm/gdp/glogs
	sudo chown -R gdp:gdp /var/swarm
	mount /dev/bigfs /var/swarm/gdp/glogs

The directory should be mode 700 or 750, owned by gdp:gdp:

	sudo chmod -R 750 /var/swarm/gdp/glogs

Mode 750 is just to allow users in the gdp group to be able
to peek into the directory.  This should be limited to
people who are maintaining the GDP.
They still won't be able to read the files unless you change
the `swarm.gdplogd.gob.mode` parameter to change the default
file mode from 0600 to something more permissive.  Note that
the umask may modify this value.

Adjust Administrative Parameters
================================

If you want to change parameters such as socket numbers or the
GOB directory you can do so without recompiling.  Configuration
files are simple "name=value" pairs, one per line.  There is
a built-in search path
"`.gdp/params:~/.gdp/params:/usr/local/etc/gdp/params:/etc/gdp/params`"
that can be overridden the `EP_PARAM_PATH` environment variable.
(Note: if a program is running setuid then only the two
system paths are searched, and `EP_PARAM_PATH` is ignored.)
Those directories are searched for files named "`gdplogd`" (for
gdplogd only) or "`gdp`" (for all programs that use the GDP).

There are many parameters you change, as described near the
end of this README.  In most cases, the ones you are likely
to find interesting (and the files in which they should
probably live) are:

`swarm.gdp.routers`			(file: `gdp`)

> A semicolon-separated list of host names or IP
addresses to search to find a running routing node.
This defaults to 127.0.0.1.  If you have a local
routing node you should name it first, followed
by "gdp-01.eecs.berkeley.edu; gdp-02.eecs.berkeley.edu"
(these are run by us for your convenience).  This
parameter is only consulted if Zeroconf fails.

`swarm.gdp.data.root`		(file: `gdp`)

> This is the root of a subtree storing persistent data, including log
data and keys.  It is the parent of the directory you created in the
previous step.

`swarm.gdplogd.log.dir`		(file: `gdplogd`)

> The name of the directory relative to `swarm.gdp.data.root` that
stores actual GDP log data.  It only applies on nodes that are running
a log daemon.  Defaults to `glogs`.

`swarm.gdplogd.gdpname`		(file: `gdplogd`)

> This is a user-friendly name for the name of the log daemon.
This is necessary so the log creation service can address a
physical log server.  It doesn't have to be publicly advertised.
If you don't specify this, the name is chosen randomly each time
`gdplogd` starts up.

### Example

In file `/etc/gdp/params/gdp`:

	swarm.gdp.routers=mygdp.example.com; gdp-01.eecs.berkeley.edu; gdp-02.eecs.berkeley.edu

This tells application
programs where to look for routers if Zeroconf fails.
In file `/etc/gdp/params/gdplogd`:

	swarm.gdp.data.root=/var/swarm/gdp
	swarm.gdplogd.gdpname=com.example.mygdp.gdplogd

This tells `gdplogd` where to store GOB log data (only needed if not
using the default, which is what is shown here) and what name to use
on startup (only needed for the `gob-create` command, but very
important to have set).

Installing and Starting the GDP Routing Daemon
==============================================

There are two versions of the router.  Version 1 is easier to
set up and run, but doesn't work through firewalls and doesn't
scale as well as Version 2.

### Version 1

> Note:
This version is deprecated; however, it can still be useful
when debugging the GDP.  In a test environment with only
one router node it requires no configuration.  If you are not
debugging the GDP itself, please move on to Version 2.

Version 1 of the router is in it's own separate repository at:
`https://repo.eecs.berkeley.edu/git/projects/swarmlab/gdp_router.git`.
You can get this the same way as the GDP base code, but use
"`gdp_router`" instead of "`gdp`".

Version 1 of the routing daemon uses a simple
"global knowledge" algorithm where every routing node knows
every name known by the system.  Names include running servers
and applications as well as logs.  Because of this, every
routing server **must** be talking with every other routing
server.

To run your own, you'll have to make sure you know the address
of all other routing nodes in the GDP.  This is a temporary
situation, but for now it is critical that all routing nodes
know about all other routing nodes.  To do this you'll have to
get the current list of nodes (`gdp-routers.list`), add your
information to that list, and make sure that list gets updated
on all the nodes.  For now we'll maintain that master list at
Berkeley.

Once you have the list of routers you'll need to start the
Python program "`gdp-router.py`".  It takes several parameters
which are described in detail in `gdp_router/README.md`.  The most
important flag is "`-r`", which takes the list of known
routers as the argument.  For example:

	gdp-router.py -r gdp-routers.list

There is no automated startup for this version of the router.
You will have to manually start it up every time the host system
reboots.

### Version 2

Version 2 of the router is in it's own separate repository at:

	repoman@repo.eecs.berkeley.edu:projects/swarmlab/gdp_router_click.git`

You can get this the same way as the GDP base code, but use
"`gdp_router_click`" instead of "`gdp`".

Version 2 has two types of nodes.  Primary nodes work much like
version 1, that is, with global knowledge of all the other
primary nodes.  This implies that Primary nodes cannot be behind
a firewall.  Secondary nodes need to talk to a Primary node,
and can be inside a firewall.

See the instructions in the `gdp_router_click` directory for
details for installing and running this code.


Start the GDP Log Daemon
========================

The program gdplogd implements physical on-disk logs.  It must
be started _after_ the routing layer.  Located in gdplogd/gdplogd,
it takes these parameters:

* `-D` _debug-spec_

> Turn on debugging.  See [Setting Debug Flags](#"Setting Debug Flags")
below for more information.  Implies `-F`.

* `-F`

> Run in foreground.  At the moment, `gdplogd` always runs
in foreground, but the intent is that it will default
to background mode without this flag.

* `-G` _gdp-addr_

> Use _gdp-addr_ as the address of the routing layer
daemon.  Defaults to `localhost:8007`.  Can also be set
with the `swarm.gdp.routers` administrative parameter.
See [Changing Parameters](#"Changing Parameters") below for more
information.

* `-N` _routing-name_

> Sets the GDP routing name, overriding the
`swarm.gdplogd.gdpname` configuration value.

* `-n` _workers_

> Start up _workers_ worker threads.  Defaults to a
minimum of one thread which can expand up to twice
the number of cores available.  Can also be set (with
finer control) using the `libep.thr.pool.min_workers`
and `libep.thr.pool.max_workers` parameters.

Creating a Log
==============

For the time being, to create your own logs you'll need the
GDP name (_not_ the DNS name) of the creation service to create
the log.  This can be set as an administrative parameter,
and the internal form is printed out as the log server starts up.
We run a creation service at Berkeley named
`edu.berkeley.eecs.gdp.service.creation`.

If you are running your own creation service, you can change
that default using the
`swarm.gdp.creation-service.name` administrative parameter, or on
the `gob-create` command line using the `-s` flag.

You'll also need to select a name for your log.  We
recommend using a name that is unlikely to clash with other
logs, either similar to the gdplogd example above or using
_institution_`.`_project_`.`_user_`.`_name_.  For example,

	edu.berkeley.eecs.swarmlab.eric.sensor23

would be a reasonable choice.  To actually create the log on
a given server, use

> `gdp-create -s` _servicename_ _logname_

where the `-s` flag is optional.  For example, if the
`swarm.gdp.creation-service.name` default value is set to
`com.example.gdp.service.creation`, the following two commands
are equivalent:

	gdp-create -s com.example.gdp.service.creation edu.berkeley.eecs.swarmlab.eric.sensor23
	gdp-create edu.berkeley.eecs.swarmlab.eric.sensor23

The good news is that this the only time you'll need to know
the name of the log server.

Full List of Administrative Parameters
======================================

Following is a list of all the administrative parameters relevant
to the GDP.  With a few exceptions mentioned elsewhere you shouldn't
have to change any of these.

* `swarm.gdp.routers` &mdash; the address(es) of the gdp routing layer
	(used by clients).  Multiple addresses can be included,
	separated by semicolons.  If no port is included, it
	defaults to 8007.  This will eventually be replaced by
	service discovery.  Defaults to 127.0.0.1:8007.

* `swarm.gdp.event.loopdelay` &mdash; if the event loop exits for some
	reason, this is the number of microseconds to delay
	before restarting the loop.  Defaults to 100000 (100msec).

* `swarm.gdp.event.looptimeout` &mdash; the timeout for the event loop;
	this is mostly just to make sure things don't "hang up"
	forever.  Defaults to 30 (seconds).

* `swarm.gdp.connect.timeout` &mdash; how long to wait for a connection
	to the GDP routing layer before giving up and trying
	another entry point (in milliseconds).  Defaults to
	10000 (ten seconds).

* `swarm.gdp.reconnect.delay` &mdash; the number of milliseconds to wait
	before attempting to reconnect if the routing layer is
	disconnected.  Defaults to 100 milliseconds.

* `swarm.gdp.invoke.timeout` &mdash; the number of milliseconds to wait
	for a response before timing out a GDP request.  Defaults
	to 10000 (ten seconds).

* `swarm.gdp.invoke.retries` &mdash; the number of times to re-send a
	request when no response is received.  Defaults to 3
	(meaning that the request may be sent up to 4 times
	total).

* `swarm.gdp.invoke.retrydelay` &mdash; the number of milliseconds
	between retry attempts.  Defaults to 5000 (five seconds).

* `swarm.gdp.crypto.key.path` &mdash; path name to search for secret
	keys when writing a log.  Defaults to
	`.:KEYS:~/.swarm/gdp/keys:/usr/local/etc/swarm/gdp/keys:/etc/swarm/gdp/keys`.

* `swarm.gdp.crypto.key.dir` &mdash; the directory in which to store the
	secret keys when creating a GOB.  Defaults to `KEYS`.
	Can be overridden by gdp-create `-K` flag.

* `swarm.gdp.crypto.hash.alg` &mdash; the default hashing algorithm.
	Defaults to `sha256`.  Can be overridden by gdp-create
	`-h` _alg_ flag.

* `swarm.gdp.crypto.sign.alg` &mdash; the default signing algorithm.
	Defaults to `ec`.  Can be overridden by gdp-create
	`-k` flag.

* `swarm.gdp.crypto.keyenc.alg` &mdash; the default secret key encryption
	algorithm.  Defaults to "aes192".  A value of "none"
	turns off encryption.  Can be overridden by gdp-create
	`-e` flag.

* `swarm.gdp.crypto.ec.curve` &mdash; the default curve to use when
	creating an EC key.  Defaults to sect283r1 (subject
	to change).  Can be overridden by gdp-create `-c` flag.

* `swarm.gdp.crypto.dsa.keylen` &mdash; the default size of an RSA
	signature key in bits.  Defaults to 2048.  Can be
	overridden by gdp-create `-b` flag.

* `swarm.gdp.crypto.rsa.keylen` &mdash; the default size of an RSA
	signature key in bits.  Defaults to 2048.  Can be
	overridden by gdp-create `-b` flag.

* `swarm.gdp.crypto.rsa.keyexp` &mdash; the exponent for an RSA signature
	key.  Defaults to 3.

* `swarm.gdp.syslog.facility` &mdash; the name of the log facility to use
	(see syslog(3) for details).  The `gdp` in the name
	can be replaced by an individual program name to set a
	value that applies only to that program, e.g.,
	`swarm.gdplogd.log.facility`.  If both are set, the more
	specific name wins.  Defaults to `local4`.

* `swarm.gdp.zeroconf.domain` &mdash; the domain searched in zeroconf
	queries.  Defaults to `local`.

* `swarm.gdp.zeroconf.enable` &mdash; enable zeroconf lookup when programs
	start up.  Defaults to `true`.

* `swarm.gdp.zeroconf.proto` &mdash; the protocol used in zeroconf queries.
	Defaults to `_gdp._tcp`.

* `swarm.gdp.data.root` &mdash; the directory in which log data will
	be stored.  Defaults to `/var/swarm/gdp`.

* `swarm.gdplogd.log.dir` &mdash; the directory that holds the actual
    data logs.  If this is not an absolute path it is relative to
	`swarm.gdp.data.root`.  Defaults to `glogs`.

* `swarm.gdplogd.advertise.interval` &mdash; how often to renew
    advertisements of known logs.  If set to zero advertisments are
	not renewed.  Defaults to 150 (seconds).

* `swarm.gdplogd.reclaim.interval` &mdash; how often to wake up to
	reclaim unused resources.  Defaults to 15 (seconds).

* `swarm.gdplogd.reclaim.age` &mdash; how long a GOB is permitted to
	sit idle before its resources are reclaimed.  Defaults
	to 300 (seconds, i.e., five minutes).

* `swarm.gdplogd.gdpname` &mdash; the name to use as the source address
	for protocol initiating from this program.  If not set,
	a random name is made up when the program is started.
	Generally speaking, you will want to set this parameter;
	I recommend reverse-dns addresses, e.g.,

	`swarm.gdplogd.gdpname=edu.berkeley.eecs.gdp-01.gdplogd`

* `swarm.gdplogd.runasuser` &mdash; the name of the UNIX account to
	switch to if gdplogd is started as root.  Generally
	it is better if gdplogd is *not* run as root; this
	parameter avoids mistakes.  If the named account does
	not exist it defaults to 1:1 (user daemon on most
	systems).

* `swarm.gdplogd.crypto.strictness` &mdash; how strictly signatures
	are enforced.  This is a sequence of comma-separated
	words where only the first letter is significant.
	Values are `verify` (signature must verify if it
	exists), `required` (signature must be present if the
	GOB has a public key), and/or `pubkeyreq` (public
	key is required).  For now, defaults to `verify`.

* `swarm.gdplogd.sequencing.allowdups` &mdash;
	allows records with record numbers that already exist to be
	written.  This effectively erases the previous value.
	**Use of this option is strongly discouraged.**
	Defaults to `false`.

* `swarm.gdplogd.sequencing.allowgaps` &mdash;
	allows gaps in record numbers.  Readers will probably get
	confused when they try to read records that do not exist.
	Defaults to `false`.

* `swarm.gdplogd.subscr.timeout` &mdash; how long a subscription will
	be kept active without being refreshed (essentially,
	the length of a "lease" on the subscription).  Defaults
	to 600 (seconds).


* `swarm.rest.kv.logname` &mdash; the name of the GOB to use for the
	key-value store.  Defaults to "`swarm.rest.kv.log`".

* `swarm.rest.prefix` &mdash; the REST prefix (e.g., `/gdp/v1/`).

* `swarm.rest.scgi.port` &mdash; the port number for the SCGI server to
	listen on.  If you change this you will also have to
	change the lighttpd configuration.  Defaults to 8001.

* `swarm.rest.scgi.pollinterval` &mdash; how often to poll for SCGI
	connections (in microseconds).  Defaults to 100000
	(100 msec).


* `libep.crypto.dev` &mdash; whether or not to try to use `/dev/crypto`
	for hardware acceleration.  Defaults to `true`.

* `libep.time.accuracy` &mdash; the value filled in for the "accuracy"
	field in time structures (defaults to zero).

* `libep.thr.pool.min_workers` &mdash; the default minimum number of
	worker threads in the thread pool.  If not specified
	the default is 1.  It can be overridden by the calling
	application.

* `libep.thr.pool.max_workers` &mdash; the default maximum number of
	worker threads in the thread pool.  If not specified
	the default is two times the number of available
	cores.  It can be overridden by the calling application.

Setting Debug Flags
===================

You can turn on debugging output using a command line flag,
conventionally "`-D`_pattern_`=`_level_".  The _pattern_ specifies
which flags should be set and _level_ specifies how much
should be printed; zero indicates no output, and more output
is added as the values increase.

By convention _level_ is no greater than 127, and values 100
and above may modify the base behavior of the program (i.e.,
do more than just printing information).

Each debug flag has a hierarchical name with (by convention)
"." as the separator, for example, "`gdp.proto`" to indicate
the protocol processing of the GDP.  The "what(1)" program on
a binary will show you which debug flags are available
including a short description.  (The `what` program is not
available on Linux; it can be simulated using `strings | grep '@(#)'`).

To enable debugging for all patterns, use "`*=`_level_", for
example "`*=18`".

<!-- vim: set ai sw=4 sts=4 ts=4 : -->
