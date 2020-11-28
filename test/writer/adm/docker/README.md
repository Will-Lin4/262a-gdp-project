% Using Docker with the GDP

**Note: This is for V2. Eventually, the V2 distinction should be dropped.**

***NOTA BENE: This is not up to date.  It's still to be done.***

# Introduction

This directory contains a very basic setup for creating docker images
for the GDP, including:

- `gdp-dev-c`, an Ubuntu 16.04-based compile environment for GDP
  applications that are written in "C".  It has the necessary packages
  and the GDP software installed.  The expectation is that application
  developers will run this directly, although it can also be
  referenced in a `FROM` command to containerize user appplications.
- `gdp-dev-python`, the equivalent for applications written in Python.
  *[[Not yet available.]]*
- `gdplogd`, for running the GDP log server.  This is described
  in more detail below.
- `gdp-router`, for running the GDP routing layer.  *[[Not currently
  implemented here --- it's actually in the gdp_router repo.]]*

There are also two images that are intended for use only from with
`FROM` commands:

- `gdp-src-base`, containing the GDP source and libraries necessary
  for compilation.  This is based on ubuntu-16.04 and hence has a
  full compiler environment.  It is very similar to `gdp-dev-c` with
  the addition of the GDP source code.
- `gdp-run-base`, containing the libraries necessary to run GDP
  applications, but not including a full Ubuntu distribution, notably
  excluding development tools.  For example, see the `Dockerfile`
  entry for `gdplogd` to see how to use this.
  _[[Right now this uses `gdp-dev-c` as the base, but that should be
  changed to use alpine.]]_


# Building Docker Images

Running `make` should be sufficient to build the standard set of
images.  Note that it builds the image but does not push them to
a registry by default (see below for details).

A `make` variable `DOCKER_TAG` is used as the tag of all of these
images.  It defaults to the version number of the GDP (as defined
in `gdp/Makefile` variables variables `GDP_VERSION_MAJOR`,
`GDP_VERION_MINOR`, and `GDP_VERSION_PATCH`).

## Arguments

There are several arguments that can be passed in when building
the Docker Image.  To pass in arguments, use:

	make DOCKERFLAGS="--build-arg VARIABLE=VALUE"

These are mostly values that will be stored in the parameter
files in the resulting image.  They can all be overridden when
starting up a container (see below for instructions).  They
are inherited from the runtime administrative parameter files
on the host system on which the image is run.

* `GDP_ROUTER` — the default GDP router.  Inherited from the
  `swarm.gdp.routers` administrative parameter.
* `GDP_CREATION_SERVICE` — The GDPname of the default log creation
  service.  Inherited from `swarm.gdp.creation-service.name` if set;
  otherwise, there is no default for now.
* `GDP_HONGD_SERVER` — The IP hostname of the Human-Oriented Name
  to GDPname Directory server (actually the MariaDB server) where
  mappings may be accessed.  At the moment there is no default,
  but there probably should be until we come up with a scalable,
  federated solution.  In the meantime, there can be only one of
  these in any GDP cluster.  Inherited from `swarm.gdp.hongdb.host`.

## Code

The code in the container comes from the current workspace.  Although
obvious personal cruft (e.g., `.BAK` files) is excluded from the
container, some files (e.g., temporary files created during testing)
may end up in the container.  For this reason, best practice for
building a production container is to check out a new image from
git and create containers from that.  For example:

~~~
    REPO="git://repo.eecs.berkeley.edu/projects/swarmlab/gdp.git"
    git clone --depth=1 $REPO gdp-docker-build
    (cd gdp-docker-build/adm/docker; make docker-push)
    rm -rf gdp-docker-build
~~~

## GDP Docker Registry

Docker images can be pushed to a registry using `make docker-push`.
The details of that repo can be set with the following `make` variables:

* `DOCKER_REG_HOST` — the host name of the registry.  Defaults to
  `gdp.cs.berkeley.edu:5005`.
* `DOCKER_REG_USER` — the user name on the registry.  Defaults to
  `gdp`.

The `docker-push` target will tag the image appropriately and then
push it to that registry.


# Running Docker Instances

## Running `gdp-dev-c` and `gdp-dev-python`

***To Be Written.***

## Running `gdplogd`

`gdplogd` has some special requirements.  In particular, it has to
have access to an external volume on which to store persistent data,
and must have a unique name.  The script `spawn-gdplogd.sh` tries
to set this up for you.

The philosophy behind `spawn-gdplogd.sh` is that the setup should
be essentially the same as it would be if `gdplogd` were not run in
a container.  For example, it examines the usual parameters files:

```
        /etc/gdp/params/gdp
        /etc/gdp/params/gdplogd
        /usr/local/etc/gdp/params/gdp
        /usr/local/etc/gdp/params/gdplogd
        $HOME/.ep_adm_params/gdp
        $HOME/.ep_adm_params/gdplogd
```

for parameters.  In particular, it uses the following parameters:

* `swarm.gdplogd.log.dir` is the Unix pathname of the directory
  holding persistent log data.  Defaults to `/var/swarm/gdp/glogs`.
* `swarm.gdp.routers` sets the default set of routers.  It is
  overridden by the `GDP_ROUTER` environment variable, which
  defaults to the value of that parameter set when the image
  was built.
* `swarm.gdp.hongdb.host` is the default name for the human-oriented
  name to GDPname directory database server.  It is overridden by
  the `GDP_HONGD_SERVER` environment variable.  It has no default.
* `swarm.gdplogd.gdpname` is the default name for the log server.
  It is overridden by the `GDPLOGD_NAME` environment variable.
  Defaults to the reversed name of the host on which the container
  is being run with `.gdplogd` appended.

If the `-v` flag is set or the `VER` environment variable is set,
the docker container invoked uses that as the version tag; otherwise
it defaults to `latest`.
