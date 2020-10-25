

GDP_ROOT = ../..
PROTO_FILE = $(GDP_ROOT)/gdp/gdp.proto

all: gdp_pb2.py

gdp_pb2.py: $(PROTO_FILE)
	protoc -I$(dir $(PROTO_FILE)) --python_out=. $(notdir $(PROTO_FILE))

clean:
	rm -f gdp_pb2.py
