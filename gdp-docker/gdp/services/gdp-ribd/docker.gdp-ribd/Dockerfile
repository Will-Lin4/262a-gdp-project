FROM mariadb:10.3

# either provide a root password via environment (-e MYSQL_ROOT_PASSWORD=mypass)
# or uncomment next line for random root password assignment (obtain from log)
# ENV MYSQL_RANDOM_ROOT_PASSWORD true


# this database declaration instigates setup of mariadb automatically
ENV MYSQL_DATABASE gdp_rib

RUN apt-get update && apt-get install -y \
	libmariadb3 mariadb-plugin-oqgraph libssl1.0.0

RUN apt-get update && apt-get install \
	--reinstall -o Dpkg::Options::="--force-confmiss" mariadb-server-10.3

RUN apt-get update && apt-get install \
	--reinstall -o Dpkg::Options::="--force-confmiss" mysql-common

# front-end binary
COPY ./gdp-ribd /

# .sql file to set up gdpr@127.0.0.1 for gdp-ribd access
COPY ./01-gdp-ribd.sql /docker-entrypoint-initdb.d/

# script to launch gdp-ribd front-end binary
COPY ./02-gdp-ribd-launch.sh /docker-entrypoint-initdb.d/
