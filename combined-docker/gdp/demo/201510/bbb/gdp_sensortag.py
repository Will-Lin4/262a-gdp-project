#!/usr/bin/env python

from sensortag import SensorTag
from KVstore import KVstore
import sys
import threading
from datetime import datetime


def workerThread(addr, log, sensors):

    def process(data):
        "data is a tuple with some binary data"
        # kv[data[0]] = (data[1], data[2])
        kv[data[0]] = data[1]

    kv = KVstore(log, KVstore.MODE_RW)

    while True:
        try:
            print datetime.now(), "Connecting to " + addr
            tag = SensorTag(addr)
            for s in sensors: tag.enable_sensor(s, 250)
            tag.start(process)
        except SensorTag.ConnectionFailure:
            print datetime.now(), "Connection lost with " + addr
            pass
            

def main(tags, sensors):
    """
    tags is a dictionary, keys being bluetooth address and values as lognames
    sensors is the list of sensors that should be enabled
    """

    threads = []

    for t in tags:
        th = threading.Thread(target=workerThread, args=(t, tags[t], sensors))
        threads.append(th)

    for th in threads:
        th.start()


if __name__=="__main__":
    # this is where we should get a little more creative.
    # for now, logname = address, and we assume that the logs are precreated

    sensors = ["optical"]
    tags = {}
    for addr in sys.argv[1:]:
        tags[addr] = "KV:"+addr

    main(tags, sensors)
