#!/usr/bin/env python

# Nitesh Mor: updated for SensorTag CC2650 (Aug 2015)
#   - works with firmware version 1.12 (Jun 23, 2015), Hardware Rev PCB 1.2/1.3
# Based on source for SensorTag ver 1.0 by Michael Saunby. April 2013
#
# Notes.
# pexpect uses regular expression so characters that have special meaning
# in regular expressions, e.g. [ and ] must be escaped with a backslash.
#
#   Copyright 2013 Michael Saunby
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

import pexpect
import sys
import struct

class SensorTag:

    """
    TI sensortag v2.0

    Provides following sensors:
        "temperature", "humidity", "barometer", "imu", "optical"

    Initialize the instance with the bluetooth address:
        example: tag = SensorTag("AA:BB:CC:AA:BB:CC")

    Then enable the desired sensors:
        tag.enable("temperature")
        tag.enable("humidity")
        ...

    You can optionally pass the time-period (in ms) between notifications from
        a particular sensor as the second argument to 'enable'.

    Then call 'start' with a callback function. The callback function
        receives a tuple t for every new notification from device, with 
            t[0] = name of the sensor
            t[1] = processed sensor reading
            t[2] = raw sensor reading as returned by the sensortag

    A simple example to just print the data is as follows
        def callback(t):
            print t

        tag.start(callback)

    """


    # each tuple is 
    #   (<data-handle>, <notif-handle>, <notif-enable-val>, <notif-disable-val>, 
    #    <config-handle>, <enable-value>, <disable-value>, <period-handle>,
    #    <min-period>, <max-period>)

    sensors = {
        "temperature" : ( 0x21, 0x22, '0100', '0000', 0x24, '01',   '00', 0x26, 300, 2550 ),
        "humidity"    : ( 0x29, 0x2A, '0100', '0000', 0x2C, '01',   '00', 0x2E, 100, 2550 ),
        "barometer"   : ( 0x31, 0x32, '0100', '0000', 0x34, '01',   '00', 0x36, 100, 2550 ),
        "imu"         : ( 0x39, 0x3A, '0100', '0000', 0x3C, '7F00', '00', 0x3E, 100, 2550 ),
        "optical"     : ( 0x41, 0x42, '0100', '0000', 0x44, '01',   '00', 0x46, 100, 2550 )
    }

    @staticmethod
    def __round(t, decimal=2):
        "Round to specified decimal places."

        tmp = "{0:."+str(decimal)+"f}"
        if isinstance(t, tuple):
            return tuple([float(tmp.format(x)) for x in t])
        else:
            return float(tmp.format(t))


    class ConnectionFailure(Exception):
        """ Raised when a connection attempt fails, or an already established
                connection fails.
            You might need to press the side power button before attempting
            to connect. SensorTag has a green light blinking every second when 
            it is in advertising mode.
         """
        pass


    def __init__(self, bluetooth_adr):

        """ 
        Initialize the sensortag instance with the bluetooth address.
        Also attempt to connect. Throws an exception if can't connect
        """

        try:

            self.con = pexpect.spawn('gatttool -b ' + bluetooth_adr + ' --interactive')
            self.con.expect('\[LE\]>', timeout=1)   # should be fairly quick
            self.con.sendline('connect')

            # testing for connection
            self.con.expect('\[CON\].*>', timeout=5)

        except pexpect.TIMEOUT:
            raise self.ConnectionFailure()

        self.data = {}


    def enable_sensor(self, s, period=0):
        """
        Enable a particular sensor. For now, we only support the notification
            mode.
        Valid sensors are: 
            "temperature", "humidity", "barometer", "imu", "optical"
        If a value for period (ms) is provided, the notification frequency is set 
            accordingly. For all the sensors, resolution is 10ms.
        """
        assert s in self.sensors

        self.char_write_cmd(self.sensors[s][1], self.sensors[s][2])
        self.char_write_cmd(self.sensors[s][4], self.sensors[s][5])

        if period!=0:
            assert self.sensors[s][8]<=period<=self.sensors[s][9]
            hex_period = "{0:#04x}".format(period/10)[2:]
            self.char_write_cmd(self.sensors[s][7], hex_period) 


    def char_write_cmd( self, handle, value ):
        cmd = 'char-write-cmd 0x%02x %s' % (handle, value)
        self.con.sendline(cmd)


    # Notification handle = 0x0025 value: 9b ff 54 07
    def start(self, callback):
        """ Start the main notification loop.
        Callback is a fuction that gets a tuple, T
            T[0] = name of the sensor
            T[1] = processed value of a sensor
            T[2] = raw data received from the sensortag
        """

        while True:
            try:
                pnum = self.con.expect('Notification handle = .*? \r', timeout=10)
            except pexpect.TIMEOUT:
                raise self.ConnectionFailure

            if pnum==0:
                after = self.con.after
                hxstr = after.split()[3:]
                handle = int(hxstr[0],16)
                val = ''.join([chr(int(n,16)) for n in hxstr[2:]])

                # now lookup the kind of sensor this value belongs to, and
                #   call the appropriate _process_* routine. FIXME
                for s in self.sensors:
                    if handle == self.sensors[s][0]:    # found it
                        func = getattr(self, "_process_"+s)
                        func(val)
                        callback((s, self.data[s], val))
                        break


    def _process_temperature(self,v):

        def sensorTmp007Convert(t):
            SCALE_LSB = 0.03125
            return (t>>2) * SCALE_LSB

        objT, ambT = tuple([sensorTmp007Convert(t) for t in struct.unpack("<HH",v)])
        self.data["temperature"] = self.__round((objT, ambT))


    def _process_imu(self,v):

        def sensorMpu9250GyroConvert(data):
            return (data * 1.0) / (65536 / 500)

        def sensorMpu9250AccConvert(data):
            t = 8   ## FIXME 
            return (data*1.0)/(32768.0/t)

        def sensorMpu9250MagConvert(data):
            return 1.0*data

        Gyro = tuple([sensorMpu9250GyroConvert(t) for t in struct.unpack("<hhh", v[0:6])])
        Accel = tuple([sensorMpu9250AccConvert(t) for t in struct.unpack("<hhh", v[6:12])])
        Mag = tuple([sensorMpu9250MagConvert(t) for t in struct.unpack("<hhh", v[12:])])

        self.data["imu"] = { "Gyro": self.__round(Gyro), 
                             "Accelerometer": self.__round(Accel), 
                             "Magnetometer": self.__round(Mag)}


    def _process_humidity(self, v):

        def sensorHdc1000Convert(rawT, rawH):
            t = (rawT/65536.0)*165 - 40
            rh = (rawH/65536.0)*100
            return t, rh

        rawT, rawH = struct.unpack("<HH", v)
        t, rh = sensorHdc1000Convert(rawT, rawH)

        self.data["humidity"] = self.__round((t, rh))
        

    def _process_barometer(self, v):

        if len(v)==6:   # struct.unpack does not understand 24 bit integers
            t = (ord(v[2])<<16) + (ord(v[1])<<8) + ord(v[0])
            p = (ord(v[5])<<16) + (ord(v[4])<<8) + ord(v[3])
        else:
            t, p = struct.unpack("<HH", v)

        def calcBmp280(data):
            return data/100.0

        self.data["barometer"] = self.__round((calcBmp280(t), calcBmp280(p)))


    def _process_optical(self, v):

        rawLux = struct.unpack("<H", v)[0]

        def sensorOpt3001Convert(data):
            m, e = (data & 0x0FFF), ((data & 0xF000)>>12)
            return m * (0.01 * pow(2.0,e))

        self.data["optical"] = self.__round(sensorOpt3001Convert(rawLux))


def main():
    """ A simple program that prints out the valiues for all the sensors """


    if len(sys.argv)<2:
        print "Usage: %s <bluetooth address>" % sys.argv[0]
        sys.exit(1)



    bluetooth_adr = sys.argv[1]
    sensors = ["temperature", "humidity", "barometer", "imu", "optical"]


    def callback(t):
        print t

    while True:
        print "Attempting to connect to", sys.argv[1]
        try:
            tag = SensorTag(bluetooth_adr)
            for s in sensors: tag.enable_sensor(s)
            tag.start(callback)
        except SensorTag.ConnectionFailure:
            print "Lost connection. Trying to reconnect"

if __name__ == "__main__":
    main()

