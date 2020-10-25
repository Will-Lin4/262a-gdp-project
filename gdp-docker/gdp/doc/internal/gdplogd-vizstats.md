% GDP Log Server Visualization Output

The GDP log server (`gdplogd`) will output messages
specifically designed for visualizing the state of the system.
It is the responsibility of other software to collect and
present this data.

***Note: this is just a first draft;
these instructions will certainly change.***

# Selecting the Destination for Visualization Messages

Visualization output is written to a destination defined by the
`swarm.gdplogd.admin.output` administrative parameter.
Possible values include:

* `none`:  No output is produced.

* `stdout`:  Write to the standard output.  Each record is prefixed
with `>#<` to make it easy to distinguish from other possible output.

* `stderr`:  Write to the standard error.  Each record is prefixed
with `>#<` to make it easy to distinguish from other possible output.

* `syslog`:  Output is sent to syslog(8).

* _N_ (where _N_ is an integer):  Output is sent to the indicated
file descriptor.

* _filename_: if nothing else matches, the value is used as a file
name which is opened for append mode.

Some of these values are output periodically (for example,
a value is output each time a log is opened, but the number of open
logs is only output periodically).  The interval for doing
periodic probes is set by the `swarm.gdplogd.admin.probeintvl`
parameter.

# Visualization Statistics Output Format

Gdplogd can optionally output statistics information
for use by administrative tools and visualizations.
These are formated as text lines in the form:

	>#<timestamp message-id n1=v1; n2=v2 ...

The `>#<` preamble is a literal string to distinguish this from
any other lines that might be output.  It is only included when the
output is to `stdout` or `stderr`.

The _timestamp_ is in ISO format and UTC timezone.

The message-id is a short string used to identify the semantics
of this statistic.
This is optionally followed by a semicolon-separated list of
[_name_`=`]_value_ pairs (the name is optional).
All _name_s and _value_s are encoded to avoid ambiguity.
For example, if a _name_ or _value_ contains `=` or `;`
that character will be encoded as `+3d` or `+3b`
respectively.

The messages are from the following list:

* `log-create`:
  Posted when logs are created.  Parameters are:

    * `log-name` &mdash; the name of the log to be created.
    * `status` &mdash; the status of the creation request.

* `log-open`:
  Posted when logs are opened.  Parameters are:

    * `log-name` &mdash; the name of the log to be opened.
    * `status` &mdash; the status of the open request.

* `log-snapshot`:
  Posted periodically as controlled by the
  `swarm.gdplogd.admin.probeintvl` parameter.
   Parameters are:

    * `name` &mdash; the name of the log.
    * `in-cache` &mdash; whether the log is in the in-memory cache.
    * `nrecs` &mdash; the number of records in the log.  Only shown
      if the log is in the cache.
    * `size` &mdash; the size of the on-disk extent files for the log.
      Only the extents currently open are included.

## Example

This shows the output from one log open and two snapshots.

    >#<2016-06-24T20:44:18.673694000Z log-open log-name=u6Uy1qETHk2ntODficWLFgNmoeKDly-qk6yQN1EcZow; status=OK
    >#<2016-06-24T20:44:28.675471000Z log-snapshot name=K5d008wsNPKURynbeh5koBMLXfZHt0iC_-VZqoLqpvA; in-cache=false
    >#<2016-06-24T20:44:28.675931000Z log-snapshot name=b5IEO6R2DM8S0cikYtB24Cqeyt0B9eatyLnn2qCl5WQ; in-cache=false
    >#<2016-06-24T20:44:28.676431000Z log-snapshot name=u6Uy1qETHk2ntODficWLFgNmoeKDly-qk6yQN1EcZow; in-cache=true; nrecs=2; size=0
    >#<2016-06-24T20:44:28.676555000Z log-snapshot name=zf-Jke7aELKuoS6vYT60XPnWhrxcGWImHEFzyU-Dyf0; in-cache=false
    >#<2016-06-24T20:44:38.673993000Z log-snapshot name=K5d008wsNPKURynbeh5koBMLXfZHt0iC_-VZqoLqpvA; in-cache=false
    >#<2016-06-24T20:44:38.674336000Z log-snapshot name=b5IEO6R2DM8S0cikYtB24Cqeyt0B9eatyLnn2qCl5WQ; in-cache=false
    >#<2016-06-24T20:44:38.674702000Z log-snapshot name=u6Uy1qETHk2ntODficWLFgNmoeKDly-qk6yQN1EcZow; in-cache=true; nrecs=2; size=0
    >#<2016-06-24T20:44:38.674815000Z log-snapshot name=zf-Jke7aELKuoS6vYT60XPnWhrxcGWImHEFzyU-Dyf0; in-cache=false
