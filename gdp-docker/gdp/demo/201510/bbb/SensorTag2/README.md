SensorTag 2.0
=============

**Use this code at your own risk.**

A simple python wrapper around BlueZ/GATTTool to talk to a TI SensorTag v2.0. 

#### Prerequisites 

To use this, first make sure that you have the appropriate software installed. I use bluez version 4.101-0ubuntu13.1 on my Ubuntu 14.04 laptop. 

Of course, you will need some bluetooth LE capable hardware too. I got a Bluetooth Adapter by Medialink from Amazon that is capable of talking to this. To check if your existing hardware supports LE: Use `hcitool dev` to see a list of bluetooth devices. You will see your devices as `hci0`, `hci0`, and so on (in case you have multiple bluetooth devices. Replace `hci0` with whatever you want to use from now on.

Executing `hciconfig -a hci0 features` should list `<LE support>`. If not, get a bluetooth USB dongle that supports LE.

Next, find the bluetooth address of your SensorTag by using `hcitool -i hci0 lescan`. It should be listed as a `CC2650 SensorTag`. This is the bluetooth address you will use for everything.

#### Usage

The sensortag library provides following sensors:
```
"temperature", "humidity", "barometer", "imu", "optical"
```

To use it in your code, initialize an instance with the bluetooth address:
```
example: tag = SensorTag("AA:BB:CC:AA:BB:CC")
```
Then enable the desired sensors:
```
tag.enable("temperature")
tag.enable("humidity")
    ...
```

Then call `start` with a callback function. The callback function
    receives a tuple `t` for every new notification from device, with 
```
t[0] = name of the sensor
t[1] = processed sensor reading
t[2] = raw sensor reading as returned by the sensortag
```
A simple example to just print the data is as follows
```
def callback(t):
    print t

tag.start(callback)
```

#### Example

To see a sample, execute `python sensortag.py <bluetooth-address>`.

#### TODO:

For now, this library only works in the notification mode, with default
sampling periods. Also, various sensor values are hard-coded. It'd be 
nicer to do service discovery using UUID's.
