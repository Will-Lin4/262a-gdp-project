We make a BeagleBone Black as a simple gateway client that talks to 
a few TI SensorTags. For this, BBB should have GDP client side code
and the appropriate scripts/packages to talk to SensorTag

Steps to be followed:

### Creation of GDP client ARM packages. 
*** This could be done on a different BBB. The goal is to get ARM compiled 
    packages, that's it. ***
1. Clone the repository to BBB, install required packages for GDP compilation.
   List of necessary packages can be found in /README
2. Use deb-pkg/package-client.sh to create a gdp-client arm package. We use
   ver 0.3-1 for now.
3. (Optionally) use lang/python/deb-pkg/package.sh to create a Python package.
   This is optional on the BBB, since this is architecture independent.


### Installation of necessary packages:
apt-get install python-pexpect

## Install relatively newer version of bluez, this needs compilation from source
## More or less copied from
## http://mike.saunby.net/2013/04/raspberry-pi-and-ti-cc2541-sensortag.html?showComment=1366548609286#c6159475368411851381

apt-get install libusb-dev libdbus-1-dev libglib2.0-dev automake \
    libudev-dev libical-dev libreadline-dev

wget https://www.kernel.org/pub/linux/bluetooth/bluez-5.4.tar.xz
tar xvf bluez-5.4.tar.xz
cd bluez-5.4
./configure --disable-systemd
make
cp attrib/gatttool /usr/local/bin/


### Get SensorTag library:
git clone https://github.com/niteshmor/SensorTag2.git

### Install the packages generated above.
This makes sure that the packages are in-fact installed in the appropriate 
location.

dpkg -i gdp-client*.deb python-gdp*.deb
apt-get -f install          ## if any dependency issues arise.

### Make sure you have a zeroconf-advertising router locally, and you have 
a log-server of your choice.

### Get the sensortag library
git clone https://github.com/niteshmor/SensorTag2.git

### Logging Sensortag Data
1. Press random buttons on the sensortag so that it is in advertisement mode
   (green LED blinking).
2. `hcitool -i hci0 lescan` will search for any advertising sensortags. Note
   the MAC address of the Sensortag.
3. Make sure you have a log created for the sensortag. I follow the convention
   "KV:<mac-address>". This is hardcoded in the provided gdp_sensortag.py
4. Make sure you also have the KVstore.py in the same directory. I need to 
   package that appropriately. This can be done by copying it from
   `/usr/share/doc/gdp/python-gdp/KVstore.py`.
5. Execute ./gpd_sensortag.py <mac-addr1> <mac-addr2> ...
