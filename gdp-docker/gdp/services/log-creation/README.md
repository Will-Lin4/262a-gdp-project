% GDP Services

A framework for running GDP services that speak the native GDP protocol, with
the boilerplate code for PDU parsing, TCP connection handling, etc.

# Log Creation Service

A service that creates logs.  It verifies that the log name is unique,
selects a server to host the log, and enters the human-oriented name
(if any) into a database pointing to the internal name.

## Parameters

**General Parameters:**

* `-v`, `--verbose`:  be quite verbose in execution.
* `-i`, `--host`:  IP name of GDP router.  Defaults to `DEFAULT_ROUTER_HOST`.
* `-p`, `--port`: IP port for GDP router.  Defaults to `DEFAULT_ROUTER_PORT`.

**Parameters for log existence/location database:**

* `-d`, `--dbname`: name of MySQL/MariaDB database.  Required.
* `-a`, `--addr`: Address(es) for this service, typically human readable
  names.  Required.
* `-s`, `--server`: Log server(s) to be used for actual log creation,
  typically human readable names.  Required.  Currently one of these
  servers will be chosen at random.

**Parameters for Human-Oriented Name to GDPname Directory database:**

* `--namedb_host`: host name of MySQL/MariaDB server.  Defaults to
  `gdp-hongd.cs.berkeley.edu`.
* `--namedb_user`: database user name.  This user name must have INSERT
  permission on the appropriate table.  Defaults to `gdp_creation_service`.
* `--namedb_passwd`: password for `namedb_user`.  No default.  See also
  `--namedb_pw_file`.
* `--namedb_pw_file`: if `namedb_passwd` is not set, read this file
  to get the password.  No default.
* `--namedb_database`: database name.  Defaults to `gdp_hongd`.
* `--namedb_table`: name of human-to-internal data table.  Defaults to
  `human_to_gdp`.
