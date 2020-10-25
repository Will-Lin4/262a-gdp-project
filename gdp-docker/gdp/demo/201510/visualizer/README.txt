
We need one system to display some fancy graphs. For now:

- Show gdp-router map and istatistics, which are located at
  http://universe.eecs.berkeley.edu:8081/
  => contact Ben for details on this.

- Show a ptolemy key-value accessor that reads data from
  the TI SensorTags and plots it in a nice form


Steps:

1. Install GUI: 
   `apt-get install ubuntu-desktop`
   Probably do a reboot afterwards

2. a) Install Java: 
      add-apt-repository ppa:webupd8team/java
      apt-get update
      apt-get install oracle-java8-installer  

      ** Need to accept the license, little hard to automate **
 
   b) Install Ptolemy (from http://chess.eecs.berkeley.edu/ptexternal/)

      svn co https://repo.eecs.berkeley.edu/svn-anon/projects/eal/ptII/trunk ptII
      cd ptII
      export PTII=`pwd`
      ./configure
      ant
      cd bin
      make
      ./vergil

3. Also install GDP client and python wrapper:
   a) create the packages using deb-pkg/package-client.sh 
      and lang/python/deb-pkg/package.sh. We use ver 0.3-1
   b) Transfer the packages to this swarmbox.
   c) `dpkg -i gdp-client*deb python-gdp*deb`


4. XXX: finish this.
