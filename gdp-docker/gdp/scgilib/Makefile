CC=		cc
PG=
WALL=		-Wall
G=		-g
O=		-O
STD=
COPTS=		${PG} ${WALL} $G $O ${STD}
CFLAGS+=	${COPTS} ${INCS}
ALL=		helloworld

all:		${ALL}

helloworld:	helloworld.o scgilib.o
	${CC} -o helloworld helloworld.o scgilib.o

scgilib.o:	scgilib.h

clean:
	-rm -f ${ALL} *.o
