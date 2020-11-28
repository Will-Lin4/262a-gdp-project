#!/bin/sh

#
#  Set up GDP environment for compilation
#
#	This is overkill if you're not compiling.
#

cd `dirname $0`/..
root=`pwd`
. $root/adm/common-support.sh

info "Setting up packages for GDP compilation."
info "This is overkill if you are only installing binaries."

info "Installing packages needed by GDP for $OS"

info "Updating the package database"
case "$PKGMGR" in
    "brew")
	brew=`which brew`
	if [ -f $brew ]; then
		brewUser=`ls -l $brew | awk '{print $3}'`
		# Only use sudo to update brew if the brew binary is owned by
		# root.  This avoids "Cowardly refusing to 'sudo brew update'"
		if [ "$brewUser" = "root" ]; then
		    sudo brew update
		else
		    brew update
		fi
	fi
	;;

    "macports")
	sudo port selfupdate
	;;

    "brewports")
	fatal "You must choose between Homebrew and Macports.  Macports suggested."
	;;
esac


case "$OS" in
    "ubuntu" | "debian" | "raspbian")
	sudo apt-get update
	sudo apt-get clean
	package build-essential
	package libevent-dev
	package libevent-pthreads
	package libsqlite3-dev
	if [ ! -f /etc/apt/sources.list.d/mariadb.list ]
	then
		package curl
		curl -sS https://downloads.mariadb.com/MariaDB/mariadb_repo_setup | sudo bash
		# clean out old cruft that gets in the way
		sudo apt remove libmariadb-client-lgpl-dev libmysqlclient-dev
	fi
	package libmariadb3
	package libmariadb-dev
	package libmariadb-dev-compat
	package zlib1g-dev
	package libssl-dev
	package uuid-dev
	package lighttpd
	package libjansson-dev
	package libprotobuf-c-dev
	package protobuf-c-compiler
	package libprotobuf-dev
	package libavahi-common-dev
	package libavahi-client-dev
	package avahi-daemon
	if [ -e /etc/systemd/system ]
	then
		package libsystemd-dev
	fi
	if ! ls /etc/apt/sources.list.d/mosquitto* > /dev/null 2>&1
	then
		package software-properties-common
		info "Setting up mosquitto repository"
		sudo apt-add-repository ppa:mosquitto-dev/mosquitto-ppa
	fi
	package libmosquitto-dev
	package mosquitto-clients
	package pandoc
	;;

    "darwin")
	package libevent
	package openssl
	package lighttpd
	package jansson
	package protobuf-c
	package sqlite3
	package mariadb-10.2
	if [ "$PKGMGR" = "brew" ]
	then
		package mosquitto

		warn "Homebrew doesn't install OpenSSL in system directories."
		info "I'll try to compensate, but it may cause linking problems."
		info "Macports does not seem to have this problem."

		warn "Homebrew doesn't support Avahi."
		info "Avahi is used for Zeroconf (automatic client"
		info "configuration.  Under Darwin, Avahi is difficult"
		info "to build without editing files.  To build gdp without"
		info "Zeroconf use 'make all_noavahi'"
	else
		package avahi
		warn "Macports doesn't support Mosquitto: install by hand."
		info "See https://mosquitto.org/ for instructions."
	fi
	package pandoc
	;;

    "freebsd")
	package libevent2
	package openssl
	package lighttpd
	package jansson
	package protobuf-c
	package avahi
	package mosquitto
	package sqlite3
	package mariadb102-client
	package hs-pandoc
	;;

    "gentoo" | "redhat")
	# untested
	package libevent-devel
	package openssl-devel
	package lighttpd
	package jansson-devel
	package avahi-devel
	package sqlite3-devel
	package mariadb102-devel
	package mosquitto
	if [ -e /etc/systemd/system ]
	then
		package systemd-dev
	fi
	package uuid-devel
	warn "Yum doesn't support Pandoc: install by hand"
	;;

    "centos")
	# untested
	package epel-release
	package libevent-devel
	package openssl-devel
	package lighttpd
	package jansson-devel
	package avahi-devel
	package sqlite3-devel
	package mariadb102-devel
	package mosquitto
	if [ -e /etc/systemd/system ]
	then
		package systemd-dev
	fi
	package pandoc
	;;

    *)
	fatal "$0: unknown OS $OS"
	;;
esac

# vim: set ai sw=8 sts=8 ts=8 :
