#!/usr/bin/env python

## See http://northernlightlabs.se/systemd.status.mail.on.unit.failure

import os
import smtplib
import argparse
import subprocess
from tempfile import TemporaryFile

def sendmail(subject, body, username=None, password=None,
                        fromaddr=None, alertaddrs=None, debug=False):
    """ A one shot wrapper for sending an email."""

    assert isinstance(subject, str)
    assert isinstance(body, str)

    host = "smtp.gmail.com"
    port = 587

    if None in [username, password, fromaddr, alertaddrs]:
        print "> Not sending email (parameters not set)\n"
        print "Subject: %s" % subject
        print "Body: %s" % body
        return

    # Add the From: and To: headers at the start!
    msg = "From: %s\r\n" % fromaddr
    msg = msg + "To: %s\r\n" % ", ".join(alertaddrs)
    # msg = msg + "MIME-Version: 1.0\r\n"
    # msg = msg + "Content-Type: text/html\r\n"
    msg = msg + "Subject: %s\r\n" % subject

    print "Sending message with %d bytes in body:\n%s" % (len(body), msg)

    msg = msg + "\r\n%s\r\n" % body


    server = smtplib.SMTP(host, port)
    if debug:
        server.set_debuglevel(1)
    server.starttls()
    server.login(username, password)
    server.sendmail(fromaddr, alertaddrs, msg)
    server.quit()



def readlines(filename):
    """returns a list of lines in filename after ignoring comments"""

    assert os.path.exists(filename)
    lines = []
    with open(filename) as fh:
         for line in fh:
            ## Ignore comments, i.e. lines that start with '#'
            _line = line.split("#")[0].strip()
            if len(_line) > 0:
                lines.append(line.strip())
    return lines

def run_cmd(cmd):
    """ 
    'cmd' is a list that is passed to subprocess.call, returns
    a nicely formatted string to be sent out in the body of the email
    """
    s = ""
    with TemporaryFile() as out, TemporaryFile() as err:
        ret = subprocess.call(cmd, stdout=out, stderr=err)
        out.seek(0)
        err.seek(0)
        s += "Running command: "
        s += " ".join(cmd) + "\n"
        s += ">>> STDOUT:\n%s\n" % out.read()
        s += ">>> STDERR:\n%s\n" % err.read()

    return s


if __name__ == "__main__":

    parser = argparse.ArgumentParser(
                        description="A mailing script for systemd units")
    parser.add_argument("-u", "--unit", type=str,
                            help="The unit that triiggered this script")
    parser.add_argument("-s", "--subject", type=str,
                            help="Subject for the email")
    parser.add_argument("-c", "--config", type=str,
                            help="A file containing email configuration. "
                                 "The file should contain exactly 4 lines "
                                 "in 'key: value' form (without quotes). "
                                 "The 4 required keys are smtp username, "
                                 "smtp password, 'From' address and a "
                                 "comma separated list of 'To' addresses. "
                                 "If omitted, no email is sent.")
    args = parser.parse_args()


    ## parse email config. we look for four fields:
    ## username, password, from, to. Except 'to', each of the fields
    ## is a single string. 'To' is interpreted as a comma separated list
    username, password, fromaddr, toaddr = None, None, None, None
    if args.config is not None:
        lines = readlines(args.config)
        for line in lines:
            __tmp = line.split(":")
            (key, val) = __tmp[0].strip().lower(), ":".join(__tmp[1:])
            assert key in ["username", "password", "from", "to"]
            if key == "username":
                username = val.strip()
            if key == "password":
                password = val.strip()
            if key == "from":
                assert "@" in val
                fromaddr = val.strip()
            if key == "to":
                __addrs = val.split(',')
                assert all(['@' in addr for addr in __addrs])
                toaddr = [addr.strip() for addr in __addrs]

    emailbody = ""

    ## Collect some logs, if possible
    if args.unit is not None:
        emailbody += "========================================\n"
        emailbody += run_cmd(["/bin/systemctl", "status", args.unit])
        emailbody += "========================================\n"
        emailbody += run_cmd(["/bin/journalctl", "-n", "50", "-u", args.unit])

    sendmail(args.subject, emailbody, username=username, password=password,
                                     fromaddr=fromaddr, alertaddrs=toaddr)

