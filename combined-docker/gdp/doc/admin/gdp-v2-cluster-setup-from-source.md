% Setting Up a Version 2 GDP Cluster From Source
%
% 2019-03-01

This describes how to set up a private GDP version 2 cluster.
It assumes that you are:

 1.  Compiling from source code.
 2.  Running on Ubuntu 16.04.  It should work on Ubuntu 18.04 but
     we haven't tested that yet, and might work on MacOS 10.13 and
     FreeBSD 11.2.  It probably will not work on Ubuntu 14.04 since
     we require systemd.
 3.  _Not_ connecting into the existing Berkeley infrastructure.
     If you are, you don't need most of these steps.

***These instructions are largely untested.  Please let us know
how they can be improved.***

Note that many of the included scripts assume you are using the
Berkeley infrastructure and reference Berkeley-specific hosts.  You
will probably have to hand edit some scripts.  A `grep -i` for
`berkeley.edu` will probably find most of them.

These instructions also assume that you are running on a fresh
Ubuntu system.

The basic outline is:

 * Gather information.
 * Get the code from the Berkeley git repository.
 * Install system packages, compile the GDP library and application
   code, and install it.
 * Set up an instance of MariaDB 10.4 and the Human-Oriented Name
   to GDPname Directory (HONGD) database.  This step uses Docker,
   but you should be able to make HONGD work in a native install.
 * Set up one or more GDP routers.
 * Set up at least one gdplogd (log data server).
 * Set up the Log Creation Service.
 * Check configuration.

Note that these instructions assume they are all being run in one
shell session.  In particular, some variables are set in one step
and then used in later steps.  If you are not installing everything
on the same machine you may need to duplicate configuration steps.

In the future we will have many of these components packaged as
precompiled Docker containers, but for now you have to compile
yourself.

# Warning about DNS Configuration

When you install Ubuntu it will create a host name in `/etc/hostname`.
However, some of this installation requires that you have a fully
qualified domain name (i.e., one with dots in it) that are known to
your DNS server.  To test this, use `hostname -f`.  If it returns
a name that contains at least one dot which ends with one of the
"well known" names such as `.com`, `.edu`, or your country code,
then everything is fine.

If it has no dots, or it ends up with `.local` or `.localdomain`
then you'll probably need to edit `/etc/hosts` to provide a real
domain name.  There are instructions available on how to do that on
the net ___which we should provide here___.

If you are not connected to any network, then names ending in
`.local` or `.localdomain` are fine.  The rest of these instructions
assume that you _are_ connected to an Internet-connected network.
Virtual machines and containers such as Docker make this distinction
fuzzy.  In most cases VMs and containers can connect to the outside
world, but the rest of the world cannot connect to them unless you
make special arrangements.  Describing these in detail depends on
what technology you are using and is beyond the scope of this
document.

# Gathering Information

Before starting, you will need to make some decisions regarding
where GDP services will be installed and run.  Things to decide:

 * Which host(s) will run GDP routers.
 * Which host(s) will be running gdplogd instances.  These systems
   should have adequate disk space to store your data.
 * Which host will run the Log Creation Service.  This can be
   a virtual machine or be run in a Docker container.
 * Which host will run the MariaDB instance used to store the
   HONGD database.  This currently runs in a Docker container.

# Fetching the Source Code

    root=`pwd`
    git clone git://repo.eecs.berkeley.edu/projects/swarmlab/gdp.git
    git clone git://repo.eecs.berkeley.edu/projects/swarmlab/gdp-router.git

# Install Packages, Compile and Install GDP Libraries

    cd $root/gdp
    adm/gdp-setup.sh
    make
    sudo make install-dev-c

# Set up MariaDB and HONGD

You need to be root or in the `docker` group before running
`gdp-mariadb-init.sh`.  You may need to be root to execute the
`mkdir` command, but it's better if you change the ownership to a
non-privileged user.  In this example we will assume that user
is `gdp:gdp` (i.e., the user name and group name are both `gdp`).

You will also need to come up with a password for the database
administrator (also known as `root`, but it is not the same as `root`
on the host system).

    rootpw=[create a new password]
    sudo mkdir -p /var/swarm/gdp/mysql-data
    sudo chown gdp:gdp /var/swarm/gdp /var/swarm/gdp/mysql-data
    cd $root/gdp/services/mariadb
    env GDP_MARIADB_ROOT_PASSWORD="$rootpw" services/mariadb/gdp-mariadb-init.sh

Once this is complete you should be able to run `adm/gdp-init-hongds.sh`
to initialize the necessary database tables:

    cd $root/gdp
    sh adm/gdp-init-hongds.sh

# Set Up GDP Router(s)

***Rick, please fill in***

    GDP_ROUTERS=[space-separated list of GDP router nodes]
    export GDP_ROUTERS
    cd $root/gdp-router
    make

# Set up GDP Log Server(s)

This should be run on each machine running `gdplogd`.  Be sure that
`GDP_ROUTERS` is set before this is run.

    adm/gdplogd-install.sh

# Set up the Log Creation Service

The `rootpw` is the root password for MariaDB which you created in the
prior step "Set up MariaDB and HONGD."

    sudo mkdir -p /etc/gdp
    (umask 037 && echo "$rootpw" | \
        sudo cp /dev/stdin /etc/gdp/creation_service_pw.txt)
    sudo chown gdp:gdp /etc/gdp /etc/gdp/creation_service_pw.txt
    cd $root/gdp/services/log-creation
    make  # generates the required protobuf bindings
    sudo mkdir -p /opt/log-creation2
    sudo cp *.py /opt/log-creation2
    sudo cp logCreationService.service \
        /etc/systemd/system/logCreationService2.service
    sudo systemctl daemon-reload

Note that you will need to have the C client library and Python bindings
installed. Additionally, you may have to satisfy the other dependencies.

    sudo apt-get install python-twisted python-mysql.connector
    sudo apt-get install protobuf-compiler python-protobuf # SEE BELOW

You do need to pay attention to the protobuf version. The default ubuntu
repositories have a very old version, which may or may not work. If you
want to install a more recent-ish version, see toward the end.

# Check Configuration

Check the runtime configuration files for rationality.

    vi /etc/gdp/params/*

# Language Bindings

These instructions only install the `C` language bindings.  If you want
other languages, look in `$root/lang` and follow the `README` files.

# MISSING ITEMS

What have I forgotten?

# Appendix: Manually installing a recent version of protobuf

*A rough outline of downloading and installing protobuf-3.5.1 (or some
similar recent-ish version) to make it work with Python.*

You need to install the compiler and associated libraries, as well as the
python bindings. Here is a rough process, please pay attention to what
you are executing instead of blindly copying and pasting.

    wget https://github.com/protocolbuffers/protobuf/releases/download/v3.5.1/protobuf-cpp-3.5.1.tar.gz
    tar xvzf protobuf-cpp-3.5.1.tar.gz
    sudo apt-get install autoconf automake libtool curl make g++ unzip
    ( cd protobuf-3.5.1 && \
      ./configure && \
      make && \
      make check && \
      sudo make install && \
      sudo ldconfig )
    wget https://github.com/protocolbuffers/protobuf/releases/download/v3.5.1/protobuf-python-3.5.1.tar.gz
    tar xvzf protobuf-python-3.5.1.tar.gz
    sudo apt-get install python-setuptools
    ( cd protobuf-3.5.1/python && \
      python setup.py build && \
      sudo python setup.py install )
