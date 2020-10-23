FROM ubuntu:xenial
RUN apt update
RUN apt install sudo git apt-transport-https ca-certificates -y
WORKDIR "/root"
RUN git clone git://repo.eecs.berkeley.edu/projects/swarmlab/gdp.git
WORKDIR "/root/gdp"
RUN adm/gdp-setup.sh
RUN make
RUN make install
RUN adm/gdp-bin-setup.sh
