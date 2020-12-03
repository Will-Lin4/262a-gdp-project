% GDP Health monitoring

A basic framework to monitor networked GDP services.

# How to?

The main executable is `monitor.py`. This contains the code for running a list
of monitors (specified in a separate file) and sending email alerts to a list of
recipients. The email configuration (SMTP credentials, recipient list, etc.) is
specified in a separate configuration file (passing secrets as command-line
parameter is bad habit).

See `python monitor.py -h` for a basic invocation, for description of monitors,
and for the required email configuration.


