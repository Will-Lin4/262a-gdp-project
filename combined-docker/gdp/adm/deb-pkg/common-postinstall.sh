#!/bin/sh

#
# Do post-installation work.  Shared with client and server.
#

set -e

# defaults
GDP_VER=2
GDP_ROOT=/usr
GDP_ETC=/etc/gdp
GDP_USER=gdp
GDP_GROUP=gdp
GDP_KEYS_DIR=$GDP_ETC/keys
GDP_LOG_DIR=/var/log/gdp
GDP_SYSLOG_FACILITY=local4
GDP_SYSLOG_LEVEL=notice
EP_PARAMS=$GDP_ETC/params

# gdp-02 and gdp-04 are not accessible to the outside world at the moment
#DEFAULT_ROUTERS="gdp-01 gdp-02 gdp-03 gdp-04"
DEFAULT_ROUTERS="gdp-01 gdp-03"

# can override values in one of these files:
{ test -r /etc/gdp.conf.sh && . /etc/gdp.conf.sh; }
{ test -r /usr/local/etc/gdp.conf.sh && . /usr/local/etc/gdp.conf.sh; }
{ test -r /etc/gdp${GDP_VER}.conf.sh && . /etc/gdp${GDP_VER}.conf.sh; }
{ test -r /usr/local/etc/gdp${GDP_VER}.conf.sh && . /usr/local/etc/gdp${GDP_VER}.conf.sh; }

umask 0022

mkdir_gdp() {
	test -d $1 && return
	if [ -e $1 ]
	then
		echo "WARNING: $1 exists but is not a directory."
		return
	fi
	echo "Creating $1 as $GDP_USER:$GDP_GROUP."
	mkdir -p $1
	chmod ${2:-0775} $1
	chown ${GDP_USER}:${GDP_GROUP} $1
}

mkfile_gdp() {
	test -f $1 && return
	if [ -e $1 ]
	then
		echo "WARNING: $1 exists but is not a file."
		return
	fi
	echo "Creating $1 as $GDP_USER:$GDP_GROUP."
	cp /dev/null $1
	chmod ${2:-0664} $1
	chown ${GDP_USER}:${GDP_GROUP} $1
}

customize() {
	sed \
		-e "s;@GDP_VER@;$GDP_VER;g" \
		-e "s;@GDP_ROOT@;$GDP_ROOT;g" \
		-e "s;@GDP_ETC@;$GDP_ETC;g" \
		-e "s;@GDP_USER@;$GDP_USER;g" \
		-e "s;@GDP_GROUP@;$GDP_GROUP;g" \
		-e "s;@GDP_LOG_DIR@;$GDP_LOG_DIR;g" \
		-e "s;@GDP_SYSLOG_FACILITY@;$GDP_SYSLOG_FACILITY;g" \
		-e "s;@GDP_SYSLOG_LEVEL@;$GDP_SYSLOG_LEVEL;g" \
		$1 > $2
	chmod ${3:=0644} $2
}

# Find domain name.  Unfortunately by default Debian doesn't seem to
# know what that is.  To do that you have to set up /etc/hosts properly,
# e.g., assuming your FQDN is host.example.com:
#	127.0.0.1	localhost
#	127.0.1.1	host.example.com host
domainname=`dnsdomainname | tr '[A-Z]' '[a-z]'`
if [ -z "$domainname" ]
then
	echo "WARNING: your DNS domain name isn't set!!!  See hosts(5) for"
	echo "information on how to do this.  I'll assume your domain is"
	echo "\"example.com\", but that may make some configuration incorrect."
	echo "You'll have to edit configurations by hand when this is fixed."
	domainname=example.com
fi
revdomain=`echo $domainname | tr '.' '\n' | tac | tr '\n' '.' | sed 's/\.$//'`

## create "gdp" group
if ! grep -q "^${GDP_GROUP}:" /etc/group
then
	echo "Creating group $GDP_GROUP."
	addgroup --system $GDP_GROUP
fi

## create "gdp" user
if ! grep -q "^${GDP_USER}:" /etc/passwd
then
	echo "Creating user $GDP_USER."
	adduser --system --ingroup $GDP_GROUP --home $GDP_ETC $GDP_USER
fi

# create directories as needed
mkdir_gdp $GDP_ETC
mkdir_gdp $GDP_KEYS_DIR 0750
mkdir_gdp $EP_PARAMS
mkdir_gdp $GDP_LOG_DIR

# now individual files
if [ ! -e $EP_PARAMS/gdp ]
then
	# determine default router set --- customized for Berkeley servers!!!
	hostname=`hostname`
	routers=`echo $DEFAULT_ROUTERS |
			tr ' ' '\n' |
			grep -v $hostname |
			shuf |
			tr '\n' ';' |
			sed -e 's/;/.eecs.berkeley.edu; /g' -e 's/; $//' `
	if echo $hostname | grep -q '^gdp-0'
	then
		routers="127.0.0.1; $routers"
	fi

	{
		echo "# The list of sites to contact to find a GDP router."
		echo '# Please add local routers at the FRONT of this list:'
		echo "swarm.gdp.routers=$routers"
	} > $EP_PARAMS/gdp
	chown ${GDP_USER}:${GDP_GROUP} $EP_PARAMS/gdp

	echo "WARNING: created new configuration at $EP_PARAMS/gdp from whole cloth."
	echo "    This is configured to use the Berkeley servers, which may"
	echo "    not be what you intended.  Check it to make sure it is"
	echo "    appropriate for your use.  Here is what I created for you:"
	echo " ----"
	cat $EP_PARAMS/gdp
	echo " ----"
fi

mkfile_gdp $GDP_LOG_DIR/gdp.log

test -f /etc/rsyslog.d/60-gdp.conf || cat > /etc/rsyslog.d/60-gdp.conf << EOF
$GDP_LOG_FACILITY.*		$GDP_LOG_DIR/gdp.log
:msg, contains, "gdplogd"	$GDP_LOG_DIR/gdplogd.log
EOF

test -f /etc/logrotate.d/gdp || cat > /etc/logrotate.d/gdp << EOF
/var/log/gdp/gdp*.log
{
	# keep 14 days worth of logs, using date in file name
	daily
	rotate 14
	dateext
	create 644 gdp gdp

	# don't rotate if file empty; compress archives
	notifempty
	compress
	delaycompress
}
EOF
