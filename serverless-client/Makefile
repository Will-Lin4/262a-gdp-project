CC = g++
FLAGS = -g -Wall -std=c++11
INCLUDES = -I./include `mysql_config --include`
LIBRARIES = -lgdp -lep -levent -levent_pthreads -lpthread -lcrypto -lprotobuf-c
FILES = dummy_client
#FILES = dummy_client gdp_buf
SRCDIR = src
OBJDIR = build
LIBROOT = libs
LIBPATHS = -Llibs -L.
SRCS = $(addprefix $(SRCDIR)/, $(addsuffix .cpp, $(FILES)))
OBJS = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(FILES)))
LIBS = $(LIBRARIES) `mysql_config --libs`
TESTOBJS = build/test.o build/gdp_buf.o

default: client

client: $(OBJS)
	$(CC) $(FLAGS) $(INCLUDES) -o dummyclient $(OBJS) $(LIBPATHS) $(LIBS)

test: $(OBJS) $(TESTOBJS)
	$(CC) $(FLAGS) $(INCLUDES) -o testrun $(TESTOBJS) $(LIBPATHS) $(LIBS)

clean:
	rm -f dummyclient testrun $(OBJDIR)/*.o

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(FLAGS) $(INCLUDES) $(LIBS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(FLAGS) $(INCLUDES) $(LIBS) -c $< -o $@
