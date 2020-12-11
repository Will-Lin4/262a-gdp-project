% Creating a docker image for running gdp-monitoring service

We treat docker as a lightweight virualization framework (and not a
software distribution mechanism). What this means is that the image is
not necessarily optimized for small size, or portability, or anything
else for that matter. 

In fact, the image contains all the code necessary to compile GDP code
from scratch. The following instructions are provided with a manual
installation of hand-selected packages. One may instead use the
`gdp/adm/gdp-setup.sh` script, which installs a few extra packages.


# Rough steps to create the image from scratch.

If you were to create an image from scratch (which should be rarely
needed), use the following steps. For normal day-to-day operations of
updating the code in a pre-existing image, see the next section.

**Note:** Treat this as merely a high-level guide (and not something to
copy/paste commands from).


- Get a docker container running.

        docker run -i -t --name gdp-monitoring ubuntu:18.04 /bin/bash

- Install `git` and other pre-requisites (all of this runs as
    root by default, so we probably don't need `sudo` at this point).

        apt-get update
        apt-get install curl git build-essential libevent-dev \
            libsqlite3-dev zlib1g-dev libssl-dev uuid-dev \
            libavahi-common-dev libavahi-client-dev libsystemd-dev \
            libjansson-dev
        curl -sS https://downloads.mariadb.com/MariaDB/mariadb_repo_setup | bash
        apt-get install libmariadb3 libmariadb-dev

        # following needed for protobuf
        apt-get install wget autoconf automake libtool curl make g++ unzip
        # install protobuf
        wget https://github.com/google/protobuf/releases/download/v3.7.0/protobuf-cpp-3.7.0.tar.gz
        tar xvzf protobuf-cpp-3.7.0.tar.gz
        ( cd protobuf-3.7.0/ && ./configure && make && make check && make install )
        # install protobuf-c; the C compiler for protobuf
        git clone https://github.com/protobuf-c/protobuf-c.git
        ( cd protobuf-c && ./autogen.sh && ./configure && make && make install )


- Clone the repository

        git clone https://repo.eecs.berkeley.edu/git-anon/projects/swarmlab/gdp.git

- Compile

        (cd gdp && make && make install-dev-c)

- Add configuration in a file `/etc/gdp/params/gdp` (system-wide).

- Install python and other dependencies for monitoring service (and a
    handful of other utilities)

        apt-get install python python-psutil iputils-ping

- Also, let's copy the code we will run from `/root/gdp/adm/monitoring`
    to a system path.

        cp -a ~/gdp/adm/monitoring /opt/
        chmod -R a+r /opt/monitoring

- Create a user `gdp` that we will use to run the actual monitoring
    service.

# Updating a pre-existing image

- First, start a container with the said image (say `gdp-monitoring`)

        docker run -i -t gdp-monitoring /bin/bash

- Make your changes in the running container
- Find the name of the running container from `docker ps`. We'll use the
    example name `nifty_ritchie`.
- From outside the container (from the host), issue

        docker commit nifty_ritchie

- You should see a sha256 hash as an output. With `docker image ls`, you
    should see an untagged image with the given hash in the `IMAGE ID`
    column. You can safely exit from the running container.
- Next, we tag the image by running following (use the first few
    characters of the sha256 hash you got earlier).

        docker tag <hash> gdp-monitoring

- Using `docker image ls` should now show the tag `gdp-monitoring`
    associated with this newly updated image.

# Running the actual service

You could do it in a few different ways. Essentially, the following
are the commands that ought to be run (assuming that we mount a
directory from the host to find secret email configuration and store
logs).

    docker run -i -t -v /var/swarm/gdp/monitoring:/mnt gdp-monitoring /bin/bash

Inside the container, run the following.

    su gdp
    python /opt/monitoring/monitor.py -s -c /mnt/emailconfig \
        -l /mnt/logs /opt/monitoring/tests.conf

To make life easier, I have created a small script in
`/docker-entrypoint.sh` inside the image. This modifies the docker
invocation above as following

    docker run -i -t -v /var/swarm/gdp/monitoring:/mnt \
        gdp-monitoring /docker-entrypoint.sh
