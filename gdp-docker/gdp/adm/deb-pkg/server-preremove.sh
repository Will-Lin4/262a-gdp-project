#!/bin/sh

set -e

SERVICE=gdplogd2

echo "Stopping the service"
service $SERVICE stop            || true
service $SERVICE disable         || true
rm /etc/systemd/system/$SERVICE.service

exit 0
