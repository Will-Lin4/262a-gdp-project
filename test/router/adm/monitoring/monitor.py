#!/usr/bin/env python

"""
A crude system monitoring script to monitor GDP systems/gateways and
send email alerts when necessary.
"""
# For the moment, the design contains a core loop that runs at a fairly
# high, but fixed frequency. Each run produces an output that is a
# potential candidate for email-alerts.
# There is at-least 1 email/day and at-most MAX_EMAILS_PER_DAY emails/day
# that are sent.


import smtplib
import hashlib
import subprocess
import datetime
import argparse
import psutil
import signal
import os
from os.path import basename
import time
import errno
from tempfile import TemporaryFile
from email.mime.application import MIMEApplication
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.utils import COMMASPACE, formatdate

LOGDIR = "/tmp/monitoring"
PERIOD = 10
TIMEOUT = 30
MAXEMAILS = 24

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


class cmdlineMonitor(object):
    """
    A monitor object that represents some details about an individual
    run of a particular test.
    """

    class NotFinished(Exception):
        pass

    def __init__(self, desc, cmd, shell=False, timeout=TIMEOUT):

        assert isinstance(desc, str)
        assert isinstance(cmd, str)
        self.desc = desc
        self.cmd = cmd
        self.shell = shell
        self.timeout = timeout

        ## following are set when the monitor is executed
        self.status = None
        self.stdout, self.stderr = None, None
        self.start_time, self.end_time = None, None

    def run(self):
        """ Runs the actual command, returns True if exit code == 0 """

        ## following works with Py2.7, 3.1 and above
        with TemporaryFile() as out, TemporaryFile() as err:

            self.start_time = time.ctime()
            __cmd = self.cmd if self.shell else self.cmd.split()
            task = subprocess.Popen(__cmd, shell=self.shell,
                                            stdout=out, stderr=err)
            __timeout = self.timeout
            while task.poll() is None and __timeout > 0:
                time.sleep(1)
                __timeout -= 1
            ## make sure the process is terminated after timeout
            if task.poll() is None:
                print "[Killing]",
                self.__kill_proc(task)

            if task.poll() is None:
                print "> ## Can't terminate %s" % str(self.cmd)

            self.end_time = time.ctime()

            out.seek(0)
            err.seek(0)
            self.status = (task.returncode == 0)
            self.stdout = out.read()
            self.stderr = err.read()

        return self.status

    @staticmethod
    def __kill_proc(task):
        """ kill the process (including children) for a given PID """

        pid = task.pid

        ## First, kill the children
        ## https://psutil.readthedocs.io/en/latest/#kill-process-tree
        parent = psutil.Process(pid)
        children = parent.children(recursive=True)

        # First ask nicely
        for p in children:
            p.send_signal(signal.SIGKILL)
        time.sleep(1)
        # Now try the heavy handed approach
        for p in children:
            try:
                p.send_signal(signal.SIGTERM)
            except (OSError, psutil.NoSuchProcess) as e:
                pass
        time.sleep(1)

        # Now, kill the parent process
        if task.poll() is None:
            task.terminate()    # ask to terminate nicely
            time.sleep(1)
            if task.poll() is None:
                task.kill()         # just kill the process
                time.sleep(1)


    def get_brief_report(self):
        """ return a one liner regarding the status"""

        if self.status is None:
            raise self.NotFinished

        tmp = "OK" if self.status else "FAILED"
        return "%s... %s" % (self.desc, tmp)


    def get_detailed_report(self):
        """ return a full report """

        if self.status is None:
            raise self.NotFinished

        s = "## Monitor: %s\n" % self.desc
        s += "## CMD: %s\n" % self.cmd
        s += "## start: %s, end: %s\n\n" % (self.start_time, self.end_time)
        s += ">>> STDOUT: \n%s\n" % self.stdout
        s += ">>> STDERR: \n%s\n" % self.stderr

        return s


class AlertMgr(object):
    """
    A 'smart' alert manager to send out emails. It makes sure that the
    number of emails sent are neither too few nor too many.
    """

    def __init__(self, maxemails, alertaddrs=None, fromaddr=None,
                        username=None, password=None, skipfirst=False):
        """
        Initialize the alert manager with rate limiting parameters,
        credentials for the smtp server, and service addresses.
        """

        self.host = "smtp.gmail.com"
        self.port = 587
        self.username = username
        self.password = password
        self.fromaddr = fromaddr
        self.alertaddrs = alertaddrs
        self.skipfirst = skipfirst

        self.last_status = None
        self.first_alert = True     # Default True, set to False on first run
        self.alert_ts = []
        self.maxemails = maxemails


    def new_alert(self, cur_status, report, attach_file=None):
        """
        Potentially send a new alert. cur_status is a list of booleans
        with the results of all tests. report is what we will send if
        we do decide to send an alert, with an optional attachment
        """

        print "> New report ready to be sent out..."

        curtime = time.time()

        ## Cleanup email alerts sent more than 24 hours ago
        while len(self.alert_ts) > 0 and curtime-self.alert_ts[0] > 86400:
            self.alert_ts.pop(0)

        ## Should we send an email alert or not?
        send_alert = False
        if len(self.alert_ts) == 0:
            send_alert = True   ## send at least one email/day
        if self.last_status is None:
            send_alert = True   ## We just started.
        if self.last_status is not None and cur_status != self.last_status:
            send_alert = True   ## we changed status
        if len(self.alert_ts) >= self.maxemails:
            send_alert = False  ## we exceeded the quota. Do not send alert
        if self.first_alert:
            self.first_alert = False
            if self.skipfirst:
                send_alert = False  ## Skip the first email alert to be sent
                self.alert_ts.append(curtime)   ## but add to alert_ts

        print "> Are conditions for sending alert satisfied? %s" % send_alert

        ## update this var to be used in next iteration
        self.last_status = cur_status

        if send_alert:
            ## Actually send the alert based on checks above.
            suffix = "ALL PASSING" if all(cur_status) else "SOME PROBLEMS"
            nonce = hashlib.md5(time.ctime()).digest().encode("hex")[:6]
            subject = "[Health monitor] Status: %s; (%d/%d, %s)" %\
                        (suffix, len(self.alert_ts)+1, self.maxemails, nonce)
            self.__sendmail(subject, report, attach_file)
            self.alert_ts.append(curtime)


    def __sendmail(self, subject, body, attach_file=None, debug=False):
        """ A one shot wrapper for sending an email."""

        assert isinstance(subject, str)
        assert isinstance(body, str)

        if None in [self.username, self.password,
                            self.fromaddr, self.alertaddrs]:
            print "> Not sending email (parameters not set)\n"
            print "Subject: %s" % subject
            print "Body: %s" % body
            return

        ## From https://stackoverflow.com/questions/3362600/

        msg = MIMEMultipart()
        msg['From'] = self.fromaddr
        msg['To'] = COMMASPACE.join(self.alertaddrs)
        msg['Date'] = formatdate(localtime=True)
        msg['Subject'] = subject
        msg.attach(MIMEText(body))

        if attach_file is not None:
            with open(attach_file, "rb") as fh:
                part = MIMEApplication(fh.read(), Name=basename(attach_file))
            part['Content-Disposition'] = "attachment; filename=%s" % \
                                                        basename(attach_file)
            msg.attach(part)


        server = smtplib.SMTP(self.host, self.port)
        if debug:
            server.set_debuglevel(1)
        server.starttls()
        server.login(self.username, self.password)
        server.sendmail(self.fromaddr, self.alertaddrs, msg.as_string())
        server.quit()


class TestSuite(object):
    """
    The overall execution framework for running individual monitors,
    rate limiting, reporting, etc.
    """


    def __init__(self, monfile, alertmgr, shell=False,
                                    logdir=LOGDIR, timeout=TIMEOUT,
                                    period=PERIOD, run_once=False):

        self.monfile = monfile      ## A file with list of (desc, cmd) tuples
        self.alertmgr = alertmgr    ## alerts
        ## set other parameters
        self.logdir = logdir
        self.period = period
        self.timeout = timeout
        self.shell = shell
        self.run_once = run_once

        ## this is where we store all the monitors
        self.monitors = None

    @staticmethod
    def __load_monitors(filename):
        """ Read a configuration file with a list of (desc, cmd) tuples """

        monitors = []   ## A list of (desc, cmd) tuples populated from a file
        for line in readlines(filename):
            __tmp = line.split("|")
            (desc, cmd) = __tmp[0].strip(), "|".join(__tmp[1:])
            monitors.append((desc, cmd))

        return monitors


    @staticmethod
    def __makedir(dirname):
        try:
            os.makedirs(dirname)
        except OSError as exc:
            if not (exc.errno == errno.EEXIST and os.path.isdir(dirname)):
                raise

    def main_loop(self):
        """ start the main loop for testing """

        while True:

            print "Loading list of monitors from %s" % self.monfile
            self.monitors = self.__load_monitors(self.monfile)

            start_time = time.time()

            t = datetime.datetime.now()
            ## setup a directory where we keep all the logs
            logdir = "%s/%d/%02d/%02d" % (self.logdir, t.year, t.month, t.day)
            logfile = "%s/%02d-%02d-%02d.txt" % (logdir, t.hour,
                                                            t.minute, t.second)
            self.__makedir(logdir)
            print "Using logfile: %s" % logfile

            ## instantiate monitors and run them
            mons = []
            for (desc, cmd) in self.monitors:
                print "Running > %s" % desc,
                mon = cmdlineMonitor(desc, cmd, shell=self.shell,
                                                timeout=self.timeout)
                mons.append(mon)
                mon.run()
                print "... %s" % ("OK" if mon.status else "FAILED")
            print ""

            ## Create and store the report
            brief_rep = self.__gen_stat_report(mons, brief=True)
            long_rep = self.__gen_stat_report(mons, brief=False)
            with open(logfile, "w") as fh:
                fh.write(long_rep)

            all_stats = [mon.status for mon in mons]
            self.alertmgr.new_alert(all_stats, brief_rep, attach_file=logfile)

            end_time = time.time()

            ## Either exit, or sleep for a given time
            if self.run_once:
                break
            time.sleep(self.period*60-(end_time-start_time))


    @staticmethod
    def __gen_stat_report(mons, brief=False):
        """
        Generate a nice status report for the tests, and returns something
        that is ready to be mailed out or stored in a file
        """
        s = "Summary (see details below)\n"
        s += "===============================================\n"
        for mon in mons:
            s += "* %s\n" % (mon.get_brief_report())
        s += "\n"

        if brief:
            return s

        s += "Details:\n"
        s += "===============================================\n"
        for mon in mons:
            s += mon.get_detailed_report()
            s += "===============================================\n"

        return s


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="A generic monitoring suite")
    parser.add_argument("-o", "--once", action="store_true",
                            help="Run once and exit")
    parser.add_argument("-s", "--shell", action="store_true",
                            help="Run commands via a shell (see Python "
                                 "subprocess module)")
    parser.add_argument("-p", "--period", type=int, default=PERIOD,
                            help="How often to run the tests (in minutes), "
                                "ignored if '-o' is set. Use something "
                                "sensible based on your monitors. "
                                "default: %d" % PERIOD)
    parser.add_argument("-t", "--timeout", type=int, default=TIMEOUT,
                            help="Timeout for a single test (in s), "
                                "default=%d s" % TIMEOUT)
    parser.add_argument("-m", "--maxemails", type=int, default=MAXEMAILS,
                            help="Max emails sent per day, "
                                "default %d" % MAXEMAILS)
    parser.add_argument("-S", "--skipfirst", action="store_true",
                            help="Don't send the first email after start")
    parser.add_argument("-c", "--config", type=str,
                            help="A file containing email configuration. "
                                 "The file should contain exactly 4 lines "
                                 "in 'key: value' form (without quotes). "
                                 "The 4 required keys are smtp username, "
                                 "smtp password, 'From' address and a "
                                 "comma separated list of 'To' addresses. "
                                 "If omitted, no email is sent.")
    parser.add_argument("-l", "--logdir", type=str, default=LOGDIR,
                            help="Log directory, default: %s" % LOGDIR)
    parser.add_argument("monitors", type=str,
                            help="A file containing the monitors that "
                                 "we run. Each line contains one monitor "
                                 "in the 'desc | cmd' form. 'desc' is a "
                                 "simple description included in summary, "
                                 "'cmd' is the actual command executed. "
                                 "Lines that start with '#' are understood "
                                 "as comments.")
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

    ## Create the alert manager
    alertmgr = AlertMgr(args.maxemails, username=username, password=password,
                            fromaddr=fromaddr, alertaddrs=toaddr,
                            skipfirst=args.skipfirst)

    ## run the suite
    suite = TestSuite(args.monitors, alertmgr, shell=args.shell,
                                    period=args.period, timeout=args.timeout,
                                    run_once=args.once, logdir=args.logdir)
    suite.main_loop()
