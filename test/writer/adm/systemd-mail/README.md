% Email notifications for service failures

Systemd can automatically restart services on failure. However, one would like
to know about such failures (rather than potentially running into a restart
loop). This directory contains a simple setup for enabling such notifications.

# How to?

Here are the steps one ought to do to setup alerts for any services:

## Part 1. Host setup

This needs to be done only once per host, regardless of how many individual
services use the email notification functionality.

0. Make sure Python (v 2.7) is installed on the host.
1. Place `systemd-mailer.py` at `/opt/systemd-mailer.py` with executable bit set.
2. Create a file `/opt/emailconfig` with the appropriate information (such as
   SMTP username, SMTP password, alert-address, etc). See
   `/opt/systemd-mailer.py -h` for details.
3. Place `unit-status-mail@.service` in `/etc/systemd/system`.
4. Run as root `systemctl enable unit-status-mail@.service`
5. Run as root `systemctl daemon-reload`

## Part 2. Service specific setup

For each service that ought to send an email notification on failure, locate
the `.service` file, and add the following under section `[Unit]`:

```
OnFailure=unit-status-mail@%n.service  
```

Afterwards, run `systemctl daemon-reload`. This is all that is needed.

## Logs?

In order to see the email notification activity for service `xyz` (associated
with file `xyz.service`), user the following (note that the two occurences of
`service` are intentional):

```
journalctl -u unit-status-mail@xyz.service.service
```


# Acknowledgements

This is based on a similar setup from
http://northernlightlabs.se/systemd.status.mail.on.unit.failure

