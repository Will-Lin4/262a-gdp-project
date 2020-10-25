To use the testing suite based on Python/GDP interface under Mac OS X:

1. Install Mac Ports: https://www.macports.org/
2. Run ../../adm/gdp-setup.sh to install the necesary packages
3. Set up pip, py-test,  then twisted:
     sudo port install py27-pip py-pytest
     sudo port select --set pip pip27
     sudo port select --set python python27
     sudo pip install twisted
4. Create a link for py.test:
     bash-3.2$ sudo -i
     ealmac23:~ root# (cd /opt/local/bin; ln -s py.test-2.7 py.test)
     ealmac23:~ root# exit
5. Run the tests:
     cd tests ./run.sh
     
