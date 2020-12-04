% Global Dataplane

This directory contains the source code for the Global Dataplane (GDP).

**NOTE WELL: This is an incomplete implementation of the GDP.  There
*will* be incompatible changes in the future.  Use in production at
your own risk, void where prohibited by law, etc., etc.  See
Implementation Status below.**

If you are a user of the GDP you probably do not want to start from
the source code.  See `README-deb.md` for installing from the Debian
packages (which includes Ubuntu).

If you are running on any system other than Debian, you have to
compile from source code.  At the time of this writing, the GDP
also compiles on MacOS, FreeBSD, and RedHat.  For details on
compiling from source, see
<https://gdp.cs.berkeley.edu/redmine/projects/gdp/wiki/Compiling_the_GDP_from_Source>
or `README-compiling.md`.  The former is probably more up to date.
If you are going to be working with the source code you should
definitely register for an account at
<https://gdp.cs.berkeley.edu/redmine/account/register>.
Unfortunately the CAPTCHA isn't sufficient to thwart bogus accounts,
so if you don't have a berkeley.edu address please contact us in
advance so we'll know to approve your account.

In most cases you will only need the client libraries and
applications.  Specifically, you do not need to run your own
routers and log servers.  If you do want to administer your own
servers, see `doc/admin/gdp-server-admin.md`.

If you are actually doing development on the GDP itself, please
see `README-developers.md` for more information.

The remainder of this document is broken into two parts.  The first
assumes you are just going to use the pre-compiled applications from
the `apps` directory.  The second assumes you are going to be
writing your own programs and linking them against the GDP library.

GDP General Use
===============

This section applies for everyone using the GDP.  It primarily
discusses configuration.

## Configuration

If you are using the servers at Berkeley you should just need to
run `adm/gdp-bin-setup.sh`.  If you are running your own servers
you can examine that script to see what is needed.  In particular,
you'll need to adjust the parameters specifying:

  * The address of the routing node(s) for clients to use to
    contact the GDP.
  * The name of the log creation service for your cluster.
  * The location of the Human-Oriented Name to GDPname Directory.

Configuration files are simple "name=value" pairs, one per line.
There is a built-in search path
"`.gdp/params:~/.gdp/params:/usr/local/etc/gdp/params:/etc/gdp/params`"
that can be overridden the `EP_PARAM_PATH` environment variable.
(Note: if a program is running setuid then only the two
system paths are searched, and `EP_PARAM_PATH` is ignored.)
Those directories are searched for a file named "`gdp`".  The
major parameters of interest are:

`swarm.gdp.routers`                (file: `gdp`)
`swarm.gdp.hongdb.host`            (file: `gdp`)
`swarm.gdp.creation-service.name`  (file: `gdp-create`)

See `man 7 gdp` (or `man gdp/gdp.7`) for details of configuration.

### Example

In file `/etc/gdp/params/gdp`:

	swarm.gdp.routers=mygdp.example.com; gdp-01.eecs.berkeley.edu; gdp-02.eecs.berkeley.edu
	swarm.gdp.hongdb.host=gdp-hongd.cs.berkeley.edu

This tells application programs where to look for routers and
where to find the Human-Oriented Name to GDPname Directory service.

## Supplied Applications

All of the supplied applications have man pages included.  This
description will just give an overview of the programs; see the
associated man pages for details.

The gdp-writer program reads records from the standard input
and writes to the target log.  It is invoked as:

	gdp-writer log-name

The _log-name_ is the name of the GCL to be appended to.  Lines are
read from the input and written to the log, where each input
line creates one log record.  See the gdp-writer(1) man page
for more details.

The gdp-reader program reads records from the log and writes
them to the standard output.  It is invoked as:

	gdp-reader [-f firstrec] [-n nrecs] [-s] [-t] [-v] log-name

The `-f` and `-n` flags specify the first record number (starting
from 1) and the maximum number of records to read.  By default
`gdp-reader` reads all the records in the log until it has returned
`nrec` records or reached the end of the log, whichever comes first.
The `-s` flag turns on subscription mode, causing `gdp-reader` to
wait until new records are added instead of stopping at the end of
the log.  It will terminate if `nrecs` records have been displayed
or if the program is interrupted.  By default `gdp-reader` assumes
that the data may be binary, so it gives a hexadecimal dump; the
`-t` flag outputs text only.  The `-v` flag prints signature
information associated with the record.  For example:

	gdp-reader -f 2 -s edu.berkeley.eecs.eric.sensor45

will return all the data already recorded in the log starting
from record 2 and then wait until more data is written; the new
data will be immediately printed.  If `-s` were used without `-f`,
no existing data would be printed, only new data.  If neither
flag were specified all the existing data would be printed, and
then `gdp-reader` would exit.

Writing GDP-based Programs
==========================

The native programming interface for the GDP is in C.  The API is
documented in `doc/developer/gdp-programmatic-api.html`.  To compile,
you'll need to use:

    -I_path-to-include-files_ `mysql_config --include`

If the GDP is installed in the standard system directories you
can skip the `-I` flag.

To link, you'll need:

    -L_path-to-libraries_ \
	-lgdp -lep -lcrypto -levent -levent_pthreads -pthread \
	`mysql_config --libs`

The `-L flag is not needed if the GDP is installed in the standard
system directories.  You may need additional `-L` flags for some
of the other packages.

[Yes, we know that we should have a `gdp-config` that acts like
`mysql_config` to simplify things.  In the meantime, see
`apps/Makefile` for a template.]

There is also a binding for Python in `lang/python` which is well
tested.  Documentation is in `lang/python/README`.  There are also
binding for Java in `lang/java` and JavaScript in `lang/js`, both
of which have not been maintained and probably do not work at the
present time.

Implementation Status
=====================

There are many functions that are not yet working.  This list
focuses primarily on items that will require incompatible changes,
at least internally (that is, recompilation may be necessary).
We hope we have the API in a reasonably good state, but we are
not ready to guarantee that there will be no more flag days.

This list is probably incomplete.

* The security model is incomplete.  The extensions for hash
chains have been started but are incomplete.  Signatures should
still work, but signatures on every append request is too slow for
a large system.

* Similarly, acknowledgements (server to client) should be signed
(or otherwise validated).  The wire protocol has been updated to
accommodate this, but it isn't implemented.

* Log names, which should ultimately be the hash of the log
metadata, are essentially random.  This means that at some point
existing logs will become inaccessible (since the naming scheme
will change).  This will require a log name directory service
(to allow human-friendly names), which in turn probably requires
the Control Plane interface.

* Log Replication has not been integrated.  This is needed for
better durability.

* Log Migration does not exist.

* Log Expiration is not implemented.  This means that all data in
all logs last forever, and logs never disappear.

* The current PDU (Protocol Data Unit) format has been radically
updated to allow future flexibility, but new features (such as
header compression) still don't exist.  It all works with the
updated router infrastructure which should be faster than the old
version, but there is still a lot to do.

* The maximum size of a PDU has been reduced in the hopes that
it would improve performance.  This means that individual writes
to the GDP can no longer be particularly large (e.g., entire
videos).  Changing this back to allow large writes would entail
another flag day.  Also, at the moment the failures if you write
an overly-large record are obscure at best.

* There is no Control Plane interface.  This means that functions
that should be automated (e.g., log placement, directory service)
must be done manually.  This may change how end users and
applications interact with the system.  The log creation service
exists, but is a complete hack and needs to be cleaned up.

* The on-disk representation has been updated to allow for richer
semantics, but performance and reliability testing remain to be
done.  It isn't clear that the new format is more compact than
the old format, but it does more.

**YOU HAVE BEEN WARNED!!!**


<!-- vim: set ai sw=4 sts=4 ts=4 : -->
<!-- Use "pandoc -sS -o README.html README.md"
to process this to HTML -->
