######################################################################
#
#  This file is appended to common/postinstall-pak to make
#  the server-specific postinstall-pak.
#
######################################################################

set -e

GDP_DATA_DIR=/var/swarm/gdp/glogs

# make sure the data directory exists
dataroot=`dirname $GDP_DATA_DIR`
mkdir_gdp $dataroot
mkdir_gdp $GDP_DATA_DIR 0750

# set up the log-server-specific runtime configuration
if [ ! -f $EP_PARAMS/gdplogd ]
then
	echo "Creating $EP_PARAMS/gdplogd."
	{
		echo "# The name of this log server (must be unique)"
		echo "swarm.gdplogd.gdpname=$revdomain.gdplogd"
		echo ""
		echo "# Run as user $GDP_USER if started up as root"
		echo "swarm.gdp.runasuser=$GDP_USER"
	} > $EP_PARAMS/gdplogd
	chown $GDP_USER:$GDP_GROUP $EP_PARAMS/gdplogd
fi

if [ -d /etc/systemd/system ]
then
	echo "Installing and enabling systemd service files."
	cat > $TMP/gdplogd.service << EOF
[Unit]
Description=GDP log daemon
After=gdp-router-click.service
Requires=network-online.target
After=network-online.target

[Service]
Type=notify
NotifyAccess=all
Environment=GDP_ROOT=$GDP_ROOT
Environment=GDP_VER=$GDP_VER
User=$GDP_USER

# give router time to start up
#ExecStartPre=/bin/sleep 20
ExecStartPre=/bin/sleep 2
ExecStart=/bin/sh $GDP_ROOT/sbin/gdplogd$GDP_VER-wrapper.sh
Restart=always

StandardOutput=syslog
StandardError=inherit
SyslogIdentifier=gdplogd
SyslogFacility=$GDP_SYSLOG_FACILITY
SyslogLevel=$GDP_SYSLOG_LEVEL

[Install]
WantedBy=multi-user.target
EOF
	cp $TMP/gdplogd.service /etc/systemd/system/gdplogd$GDP_VER.service
	rm $TMP/gdplogd.service
#	if $GDP_REST_INSTALL
#	then
#		customize adm/gdp-rest.service.template /etc/systemd/system/gdp-rest.service
#	fi
	systemctl daemon-reload
	systemctl enable gdplogd$GDP_VER
#	if $GDP_REST_INSTALL
#	then
#		systemctl enable gdp-rest
#		echo "WARNING: Startup scripts for gdp-rest are installed, but"
#		echo "you will need to configure a web server to use the SCGI"
#		echo "interface.  See README-CAAPI.md for advice."
#	fi
else
	echo "WARNING: No system initialization configured"
fi
