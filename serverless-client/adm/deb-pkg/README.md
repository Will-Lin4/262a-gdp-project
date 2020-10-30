% DEBIAN PACKAGE BUILDS

This directory builds three Debian packages:

* `gdp-client` for users installing other provided clients such as
  those found in `gdp-if`: shared libraries and applications.
* `gdp-dev-c` for C developers: include files, libraries, etc. as
  well as everything in `gdp-client`.
* `gdp-server`: `gdplogd` and anything else necessary.

When installing on a new Ubuntu system:

```
	sudo dpkg -i gdp-<package>.deb
	sudo apt-get install -f
```


# Package Descriptions

## gdp-server

Provides `gdplogd`.

Requires gdp-client.

_In the future might provide `gdp-hongd` and `log-creation-service`.
[Those are arguably a different package.]_

## gdp-dev-c

Everything to do application development in C.

Provides all development libraries, include files, etc.

Requires gdp-client.


## gdp-dev-python

Everything to do application development in Python.

Provides ??? (Python language bindings).


## gdp-client

Provides `libep.${VER}.so`, `libgdp.${VER}.so`, most apps.

Does not contain include files or server.


## gdp-doc

Provides documentation.  Unclear if this is targetted to developers,
users, admins, whatever.

Requires `pandoc`?

_[Doesn't exist yet.]_


## gdp-mqtt-gateway (in `gdp-if` repo)

Provides `mqtt-gdp-gateway`.

Requires `gdp-client`.

_[Doesn't exist yet.]_
