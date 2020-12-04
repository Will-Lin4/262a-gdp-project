% Installing a Global Dataplane Server From Source Code

These instructions are relevant *only* if you are installing the
server code from the source tree.  If you have installed the
server code from a package, or if it is already installed, read
the document `doc/gdp-server-admin.md`.

Generally speaking, we are moving toward distributions using Docker
images (first choice) and Debian packages.  Installations from
source should only be used if you are collaborating with us on the
base system.

These steps assume you are building and installing the code from the
source tree.  If you are using a Debian package, see the instructions
with it and skip to the next section.

Installing Step by Step
=======================

  * Run the script `adm/gdp-setup.sh` to set up your system
    so that the source code can be compilied.  In particular,
	this installs all the third party packages you should need.
  * Create `/usr/local/etc/gdp.conf.sh` or `/etc/gdp.conf.sh`
    if you want to override any of the defaults.
  * Run the script `adm/init-gdp-server.sh` from the root of
    the GDP source tree.  This creates the `gdp` user, creates
	the system directories, etc.  Note that this has certain
	dependencies on Berkeley.  See below for details.
  * Compile the source tree using `make` from the source root.
    Details can be found in `README-compiling.md`.
  * Install the header files, libraries, and binaries using
    `make install`.  You will probably need to be root for
	this to work.  If you changed the setting for `GDP_ROOT`
	in `gdp.conf.sh` you will also have to set `LOCAL_ROOT`
	on the command line, e.g., if you have set `GDP_ROOT` to
	/usr/local/gdp, use:

		make install LOCAL_ROOT=/usr/local/gdp

Details
=======

## Changing Defaults

If you want to change defaults, create `/usr/local/etc/gdp.conf.sh`
or `/etc/gdp.conf.sh`.  This is read as a shell file, and should
consist of NAME=value definitions.  There are several parameters
that can be changed, the most useful of which is `GDP_ROOT`.

There are three cases for `GDP_ROOT`:

  * `GDP_ROOT` = `/usr` means that everything will be installed
    in the "normal" system directories: `/usr/bin`, `/usr/sbin`,
    `/var/log`, and `/etc/gdp`.  Note that files in `/etc` are in
    a subdirectory.  This is the default.
  * `GDP_ROOT` = `/usr/local` or `/opt/local` means that everything
    will be installed in subdirectories of `GDP_ROOT` with the
    exception of configuration files, which will be stored in
    `GDP_ROOT/etc/gdp`.
  * Otherwise everything is installed in subdirectories of
    the indicated directory.  For example, if `GDP_ROOT` is
    `/home/gdp` then all installation will go into that
    directory instead of the system directories, i.e.,
    `/home/gdp/bin`, `/home/gdp/etc`, etc.

Other locations can be set explicitly (rather that deriving from
`GDP_ROOT`):

  * `GDP_ETC`: the location for configuration files.
  * `GDP_LOG_DIR`: where system logs are stored.

## Initializing the Server

The script `adm/init-gdp-server.sh` should do most of the steps
necessary for initializing a server.  This:

  * Creates a user and group named `gdp:gdp` if it doesn not exist.
  * Creates directories needed by the GDP.  See below for details.
  * Creates initial parameter files.

This script does have some Berkeley dependencies that may require
editing some files after you have run the script.  These will
be located in the directory `$GDP_ETC/params`.

The parameter `swarm.gdp.routers` (file `gdp`) is set to the
list of hosts to search to find a GDP router.  This will be
initialized with the names of routers at Berkeley.  If you are
running your own router you will probably want to change this.

The parameter `swarm.gdplogd.gdpname` (file `gdplogd`) assumes
that your domain name is "eecs.berkeley.edu".  You should change
this to match your domain.  The convention is to use the
reversed domain name of the host running `gdplogd` with
`.gdplogd` appended.  For example, if your `gdplogd` server
is named `server.foo.example.com`, the value of that parameter
should be `com.example.foo.server.gdplogd`.

Next Steps
==========

Proceed to the documenet `doc/gdp-server-admin.md` for details about
post-install operation.

<!-- vim: set ai sw=4 sts=4 ts=4 : -->
<!-- Use
	pandoc -s -o README-server-install.html README-server-install.md
to process this to HTML -->
