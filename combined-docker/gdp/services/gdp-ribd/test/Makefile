gdp-nhop:  gdp-ribd-client.c ../gdp-ribd.h
	gcc -Wall -o gdp-nhop gdp-ribd-client.c \
		-I../../../../gdp ../../../../gdp/libs/libgdp.a \
		-I../../../../ep ../../../../gdp/libs/libep.a \
		-lmariadb -lpthread -lcrypto

all: gdp-nhop

clean:
	rm gdp-nhop
