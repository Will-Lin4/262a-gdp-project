% GDP Routing Information Base (RIB)

The GDP v2 Network design uses a two-layer routing and forwarding
(switching) framework, similar to modern SDN architectures. The GDP
Routing Information Base (RIB) is the routing layer for a GDP Trust
Domain.

The GDP RIB Daemon receives network adjacency advertisements and
withdrawals from GDP Routers, maintains an adjacency table and a
dynamic graph viewport on that adjacency table in MariaDB/OQGraph,
responds to forwarding layer requests, and expires adjacencies which
have not been readvertised recently nor withdrawn. The forwarding
layer can ask the GDP RIB to either find (anycast) or mfind
(multicast) the shortest weighted path (anycast) or paths (multicast)
from the current location (request origin) to a destination and expect
a response containing the derived next hop (anycast) or hops (sorted
by locality for multicast) or no route.

***Note: A "SINGLE_ROUTER true" router configuration will never
   attempt to connect to a gdp-ribd, so gdp-ribd installation is not
   required in that special case.***

The following instructions are specific to Ubuntu 16.04 and MariaDB 10.3

# GDP RIB Daemon

## Step 1: Set up the GDP Repository

Clone the gdp repository, invoke the gdp-setup script, then invoke
make from the workspace root:

```
    $ git clone repoman@repo.eecs.berkeley.edu:projects/swarmlab/gdp.git gdp
    $ cd gdp
    $ ./adm/gdp-setup.sh
    $ make
    $ cd services/gdp-ribd
```
## Step 2: Site Specific Password

The GDP RIB's `gdp_rib_user` database account is configured for
`127.0.0.1` (loopback) access only, which should be fairly secure on a
dedicated host or docker container. If desired, the default account
password can be replaced with a site specific password, by editing
gdp-ribd.sql (search for `IDENTIFIED BY`) and gdp-ribd.c (search for
`#define IDENTIFIED_BY`) to replace the default `gdp_rib_pass`
password string with a site specific secret, before building
gdp-ribd. Likewise, the account username could be changed. Clearly,
this is not ideal security (given strings are accessible from
binaries, so chmod 700 gdp-ribd if exposed to other users) but the
current state will have to suffice for the interim implementation.

## Step 3: Build the GDP RIB Daemon

```
	$ make gdp-ribd
```

## Step 4: Select Installation Type

The GDP RIB Daemon and MariaDB Server with OQGraph Engine plugin are
installed together, either directly on Linux or in Docker. Procedures
for each installation type are described in the following sections.

# GDP RIB Installation on Linux

## Step 5-on-Linux: MariaDB Installation

Install the following additional packages, and supply a new password
for the MariaDB server "root" user when prompted.

```
    $ sudo apt install mariadb-server mariadb-plugin-oqgraph
```	

Check for the existence of the following MariaDB configuration files,
which may not be present if MariaDB has never been installed on this
host:

```
	/etc/mysql/conf.d/mysqld_safe_syslog.cnf
	/etc/init.d/mysql
	/etc/mysql/debian-start
	/etc/mysql/my.cnf
```

If the configuration files listed above do ***NOT*** exist, install them:

```
	$ sudo apt install \
		--reinstall -o Dpkg::Options::="--force-confmiss" mariadb-server-10.3
	$ sudo apt install \
		--reinstall -o Dpkg::Options::="--force-confmiss" mysql-common
```

Start MariaDB, then secure the installation for production use:

```
    $ sudo systemctl enable mariadb
    $ sudo systemctl start mariadb
	$ sudo mysql_secure_installation
```	

## Step 6-on-Linux: MariaDB Setup

Install the oqgraph engine, account settings, database, tables, and
stored procedures into MariaDB:

```
    $ sudo mysql -p -v --show-warnings < gdp-ribd.sql
	password: <mariadb_root_password>
```

***Note: gdp-ribd.sql includes some unit tests and will empty any
   existing RIB tables (i.e. it is RIB data destructive).***

## Step 7-on-Linux: GDP RIB Installation

Install and start gdp-ribd as a systemd service, and confirm status is good:

```
	$ sudo make install
	$ sudo systemctl status gdp-ribd
```

The GDP RIB on Linux is ready for use in a GDP Trust Domain, listening
for GDP Router communication on UDP port 9001.

# GDP RIB Installation on Docker

## Step 5-on-Docker: Build GDP RIB Docker Container

```
	$ make docker
```
## Step 6-on-Docker: Run GDP RIB in Docker

```
	$ docker run --network=host -e MYSQL_ROOT_PASSWORD=mypass gdp-ribd:latest
```

***Note: mysql_secure_installation usage is not YET supported, but if
   invoked within the container instance, be careful not to disable
   remote root access as 'root'@'localhost' will be unavailable and
   there is no 'root'@'127.0.0.1' account in the MariaDB docker base
   image by default.***

The GDP RIB on Docker is ready for use in a GDP Trust Domain, listening
for GDP Router communication on UDP port 9001.

## Step 7-on-Docker: Database Access

From a "docker exec -it <id> bash" attachment to the gdp-ribd:latest
id, run:

```
	root@dockerhost:/# mysql -p
```

...and provide the password listed in the "docker run" display output
(search for "GENERATED ROOT PASSWORD:"). The root password can be
changed using the `mysql_secure_installation` script, with the caveat
noted above.

