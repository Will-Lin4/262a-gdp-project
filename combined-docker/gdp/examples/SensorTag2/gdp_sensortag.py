#!/usr/bin/env python

from sensortag import SensorTag
from KVstore import KVstore
import sys
import threading
import pprint
from datetime import datetime
import ConfigParser
import time


def workerThread(addr, kvlog, sensor_dict):
    """
    This function gets called as soon as we create new threads.
        addr: Bluetooth address of the sensortag this thread should process
        kvlog: a log that backs the Key-value store we will store data in
        sensor_dict: a dictionary with name of sensors as keys and
                     measurement frequency as values.
    """

    def process(data):
        "data is a tuple with some binary data"
        # kv[data[0]] = (data[1], data[2])
        kv[data[0]] = data[1]

    kv = KVstore(kvlog, KVstore.MODE_RW)

    while True:
        try:
            print datetime.now(), "Connecting to " + addr
            tag = SensorTag(addr)
            for k in sensor_dict.keys():
                tag.enable_sensor(k, sensor_dict[k])
            tag.start(process)
        except SensorTag.ConnectionFailure:
            print datetime.now(), "Connection lost with " + addr
            pass


def main(configfile):
    """
    Read the config file and create an appropriate sensortag object
        for each sensortag. We use one thread for each sensortag object
    """

    config = ConfigParser.ConfigParser()
    config.read(configfile)

    sections = config.sections()
    threads = []

    valid_sensors = ["temperature", "humidity", "barometer", "imu", "optical"]

    for section in sections:

        addr = config.get(section, "mac")
        kvlog = config.get(section, "kvstore")
        sdict = {}
        for s in valid_sensors:
            try:
                sdict[s] = config.getint(section, s)
            except ConfigParser.NoOptionError:
                pass

        th = threading.Thread(target=workerThread, args=(addr, kvlog, sdict))
        threads.append(th)

    for th in threads:
        th.start()
        time.sleep(1)


if __name__=="__main__":
    # this is where we should get a little more creative.
    # for now, logname = address, and we assume that the logs are precreated

    if len(sys.argv)<2:
        print "Usage: %s <config-file>" % sys.argv[0]
        sys.exit(1)

    main(sys.argv[1])
