% Setting up MariaDB for the GDP

NOTE WELL: These services do not actually work in a Docker container
for a variety of reasons.  They are merely a clue for the right path.
We'll get them automated as we can, but they were originally designed
to work on a raw system rather than in Docker, and the conversion has
not been completed.

___Do not install this service if you already have a native
(non-Dockerized) version of MariaDB or MySQL running on your host.___

MariaDB is used to store the Human-Oriented Name to GDPname Directory
(HONGD) and may be used for other purposes in the future.  In our
configuration it runs in a Docker container.

This code runs on the host for the Docker container.  It starts up a
Docker container that in turn starts up MariaDB.

We use a publicly supported Docker image (mariadb:10.4).  Everything in
that image should already be set up once we initialize the database.

These instructions do not initialize the GDP-specific databases.
For that, see `.../adm/gdp-init-hongds.sh`, where `...` is the root
of the GDP source tree.

# Initial Setup

To do the initial setup, use the following steps:

1.  Create the data directory.  By default this is
    `/var/swarm/gdp/mysql-data`, but this can be changed by setting
    the `GDP_MARIADB_DATADIR` parameter (see below).

    ```sh
	GDP_MARIADB_DATADIR=/var/swarm/gdp/mysql-data
	GDP_MARIADB_USER=gdp
    	mkdir -p $GDP_MARIADB_DATADIR
	chown $GDP_MARIADB_USER $GDP_MARIADB_DATADIR
    ```

    The `GDP_MARIADB_USER` should probably be a dedicated user name.
    The MariaDB daemon runs as this user.

2.  Select a root password for MariaDB.

    ```sh
    	GDP_MARIADB_ROOT_PASSWORD=<your-choice>
    ```

    Be sure you pick a good password; don't just copy-paste this one!

3.  Run the `gdp-mariadb-init.sh` script in this directory.  This will
    install a systemd service file and a helper script to start up
    MariaDB on reboot,

# Changing Defaults

You can change the location of the data file by setting the
`GDP_MARIADB_DATADIR` variable in `/etc/gdp.conf.sh`.  The default
is `/var/swarm/gdp/mysql-data`.

The default value for `GDP_MARIADB_USER` is the owner of
`$GDP_MARIADB_DATADIR`.  If for some reason you need to change
this you can set it in the same file.

You should ___not___ set `GDP_MARIADB_ROOT_PASSWORD` in the
configuration file.  Keep this in a safe place in case you need it
for database maintenance.
