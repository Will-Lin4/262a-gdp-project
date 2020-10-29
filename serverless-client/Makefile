CC = g++
FLAGS = -g -Wall
INCLUDES = -I./include `mysql_config --include`
LIBRARIES = -lgdp -lep -levent -levent_pthreads -lpthread -lcrypto -lprotobuf-c
FILES = dummy_client gdp_buf
SRCDIR = src
OBJDIR = build
LIBROOT = libs
LIBPATHS = -Llibs -L.
SRCS = $(addprefix $(SRCDIR)/, $(addsuffix .cpp, $(FILES)))
OBJS = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(FILES)))
LIBS = $(LIBRARIES) `mysql_config --libs`

default: client

client: $(OBJS)
	$(CC) $(FLAGS) $(INCLUDES) -o dummyclient $(OBJS) $(LIBPATHS) $(LIBS)

clean:
	rm -f dummyclient $(OBJDIR)/*.o

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(FLAGS) $(INCLUDES) $(LIBS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(FLAGS) $(INCLUDES) $(LIBS) -c $< -o $@
