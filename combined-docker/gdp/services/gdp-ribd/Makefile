GDP_SRC_ROOT=	../..
GDP_DOC_SRC=	${GDP_SRC_ROOT}/doc
GDP_LIBS_SRC=	${GDP_SRC_ROOT}/libs

ALL=		gdp-ribd

.PHONY: install-service install-binary install start stop reinstall deinstall

gdp-ribd: gdp-ribd.c gdp-ribd.h
	gcc -Wall -o gdp-ribd gdp-ribd.c \
		-I/usr/include/mariadb -lmariadb \
		-I${GDP_SRC_ROOT} -L${GDP_LIBS_SRC} -lgdp -lep \
		-lpthread -lcrypto

install-service:
	sudo cp gdp-ribd.service /etc/systemd/system/
	sudo systemctl enable gdp-ribd
	@echo "Installed service files:"
	@ls -lsa /etc/systemd/system/gdp-ribd.service
	@ls -lsa /etc/systemd/system/multi-user.target.wants/gdp-ribd.service

install-binary: gdp-ribd
	sudo cp -p gdp-ribd /usr/sbin/gdp-ribd
	sudo chmod 700 /usr/sbin/gdp-ribd
	sudo chown gdp:gdp /usr/sbin/gdp-ribd
	@echo "Installed binary:"
	@ls -lsa /usr/sbin/gdp-ribd

install: install-binary install-service start

all: gdp-ribd

docker: gdp-ribd
	cp -p ./gdp-ribd ./docker.gdp-ribd/
	cp -p ./gdp-ribd.sql ./docker.gdp-ribd/01-gdp-ribd.sql
	cp -p ./gdp-ribd-launch.sh ./docker.gdp-ribd/02-gdp-ribd-launch.sh
	(cd ./docker.gdp-ribd/ && docker build -t gdp-ribd . )

clean:
	rm ${ALL}

status:
	sudo systemctl status gdp-ribd

start:
	sudo systemctl start gdp-ribd

stop:
	sudo systemctl stop gdp-ribd

reinstall: stop install-binary start

deinstall:
	sudo systemctl stop gdp-ribd
	sudo systemctl disable gdp-ribd
	sudo rm -f /etc/systemd/system/gdp-ribd.service
	sudo rm -f /usr/sbin/gdp-ribd

PANDOC=		pandoc
PANFLAGS=	-s -c ${GDP_DOC_SRC}/gdpdoc.css --self-contained

.SUFFIXES: .md .html .pdf

.md.html .md.pdf:
	${PANDOC} ${PANFLAGS} -o $@ $<
