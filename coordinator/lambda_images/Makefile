CC = g++
FLAGS = -g -Wall -std=c++17
INCLUDES = -I../include -I/usr/include -I../../mplambda/include -I/usr/local/include -I/usr/local/include/eigen3
LIBRARIES = -lgdp -lep -levent -levent_pthreads -lpthread -lcrypto -lprotobuf-c
FILES = test_local_compute test_local_aggregator
SRCDIR = src

default: $(FILES)

test_local_compute:
	$(CC) $(FLAGS) -o test_local_compute test_local_compute.cpp $(LIBRARIES) $(INCLUDES)

test_local_aggregator:
	$(CC) $(FLAGS) -o test_local_aggregator test_local_aggregator.cpp $(LIBRARIES) $(INCLUDES) 

clean:
	rm -f $(FILES)

