% Human-Oriented Name to GDPname Directory (HONGD)

This directory contains support files for HONGD.

***THE CONTENTS OF THIS DIRECTORY ARE NOT FINISHED.  PLEASE DO
DO NOT USE THIS.  IN THE MEANTIME, USE `adm/gdp-init-hongds.sh`.***

# Background

The Human-Oriented Name to GDPname Directory translates names
that humans understand (e.g., `edu.berkeley.eecs.eric.test.001`)
to internal, binary names that are actually the hash of the
metadata for the GDP Object (GOB) associated with that name.
These are normally represented in print as a modified Base64-encoded
string, e.g., `az9hLzcwGDj0s6pwmgQFPUKnepLXiU_IoHk1FCC2_8k`.

Previous versions of the GDP just used the SHA256 hash of the
human-oriented name as the GDPname, but GDPv2 needs a database
to store the translation.

# Installation and Operation

The current implementation of HONGD uses a MariaDB relational database
to store the mappings (MySQL should work as well, but we opt for
MariaDB because of licensing issues).  Applications contact the
database server directly to read the database.  Writing is handled
through a Log Creation Service (contained in a different source
directory).

The MariaDB database engine can be run either under Docker or
natively.  Note that if you have an installed version of MariaDB
or MySQL running on the standard port (3306) you _must_ use the
_Native Host_ instructions.

## Running under Docker

To run a pre-built docker image from the Berkeley repository:

	make docker-pull docker-install

To build and install a docker image:

	make docker-build docker-install

## Running on a Native Host

	make native-install

## Running on an Alternate Port

To run HONGD on an alternate IP port you will have to reconfigure
the Docker container and all applications.

To reconfigure the server port in Docker, edit `/etc/gdp.conf.sh`
(or `/usr/local/etc/gdp.conf.sh`) and add:

	GDP_HONGD_PORT=3307

(with `3307` being a port of your choice).

To reconfigure the port for applications you will have to edit the
file `/etc/gdp/params/gdp` on all hosts that run GDP applications
and add the string `:3307` to the line beginning `swarm.gdp.hongdb.host=`,
e.g.,

	swarm.gdp.hongdb.host=hongdb.example.com:3307

# Back Compatibility

Some GDPnames still use the old mechanism of using a SHA256 hash
of the human name as the internal name.  If you are running with
debugging on you may see warnings about this.  They can be ignored,
or you can use the `gdp-name-add` and `gdp-name-xlate` applications
to insert an explicit mapping.  Assuming `$hname` is set to the
human-oriented name:

	b64name=`gdp-name-xlate -b $hname`
	gdp-name-add $hname $b64name

# Future Changes
