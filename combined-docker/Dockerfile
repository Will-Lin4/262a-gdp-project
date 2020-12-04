FROM ubuntu:bionic
WORKDIR /root/
RUN apt update && \
    apt install -y git build-essential cmake libassimp-dev libatomic1 pkg-config libcurl4-openssl-dev zlib1g-dev libssl-dev

RUN apt update && apt install sudo git apt-transport-https ca-certificates -y
COPY gdp /root/gdp
WORKDIR /root/gdp
RUN adm/gdp-setup.sh
RUN make
RUN make install
RUN adm/gdp-bin-setup.sh

COPY eigen-3.3.7 eigen
WORKDIR eigen
RUN mkdir build
WORKDIR build
RUN cmake ../
RUN make install
WORKDIR /root

COPY libccd libccd
WORKDIR libccd
RUN mkdir build
WORKDIR build
RUN cmake ..
RUN make -j 8
RUN make install 
WORKDIR /root

COPY fcl fcl
WORKDIR fcl
RUN mkdir build
WORKDIR build
RUN cmake ..
RUN make -j 8
RUN make install
WORKDIR /root

COPY nigh nigh

#uncomment below for aws lambda support
#also move all files in aws-sdk into mplambda-docker folder

#COPY aws-lambda-cpp aws-lambda-cpp
#WORKDIR aws-lambda-cpp
#RUN mkdir build
#WORKDIR build
#RUN cmake .. -DCMAKE_BUILD_TYPE=Release
#RUN make -j 8
#RUN make install
#WORKDIR /root
#
#COPY aws-c-common aws-c-common
#WORKDIR aws-c-common
#RUN mkdir build
#WORKDIR build
#RUN cmake .. -DCMAKE_MODULE_PATH=/usr/local/lib/cmake/
#RUN make -j 8
#RUN make install
#WORKDIR /root
#
#COPY aws-checksums aws-checksums
#WORKDIR aws-checksums
#RUN mkdir build
#WORKDIR build
#RUN cmake .. -DCMAKE_MODULE_PATH=/usr/local/lib/cmake/
#RUN make -j 8
#RUN make install
#WORKDIR /root
#
#COPY aws-c-event-stream aws-c-event-stream
#WORKDIR aws-c-event-stream
#RUN mkdir build
#WORKDIR build
#RUN cmake .. -DCMAKE_MODULE_PATH=/usr/local/lib/cmake/
#RUN make -j 8
#RUN make install
#WORKDIR /root
#
#COPY aws-sdk-cpp aws-sdk-cpp
#WORKDIR aws-sdk-cpp
#RUN mkdir build
#WORKDIR build
#RUN cmake .. -DBUILD_ONLY="s3;lambda" -DENABLE_UNITY_BUILD=ON -DCMAKE_BUILD_TYPE=Release
#RUN make -j 8
#RUN make install
#WORKDIR /root
#
#RUN rm -rf aws-c-common
#RUN rm -rf aws-c-event-stream
#RUN rm -rf aws-checksums
#RUN rm -rf aws-lambda-cpp
#RUN rm -rf aws-sdk-cpp
#RUN rm -rf eigen
#RUN rm -rf fcl
#RUN rm -rf libccd

COPY mplambda mplambda
WORKDIR mplambda
RUN mkdir build
WORKDIR build
#uncomment below for aws lambda support and replace the uncommented cmake command
#RUN cmake ..
RUN cmake -DAPPLE=1 ..
RUN make -j 8 

