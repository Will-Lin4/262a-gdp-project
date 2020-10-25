FROM ubuntu:xenial
RUN apt update && apt install sudo git apt-transport-https ca-certificates -y
COPY gdp /root/gdp
WORKDIR /root/gdp
RUN adm/gdp-setup.sh
RUN make
RUN make install
RUN adm/gdp-bin-setup.sh
