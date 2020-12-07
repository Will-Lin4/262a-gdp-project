CC = g++
FLAGS = -g -Wall -std=c++17
INCLUDES = -Iinclude -I/usr/include -I../mplambda/include -I/usr/local/include -I/usr/local/include/eigen3
LIBRARIES = -lgdp -lep -levent -levent_pthreads -lpthread -lcrypto -lprotobuf-c
FILES = example_coordinator example_client
OBJDIR = build
SRCDIR = src
SRCS = $(addprefix $(SRCDIR)/, $(addsuffix .cpp, $(FILES)))
OBJS = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(FILES)))

default: coordinator client

coordinator: $(OBJS)
	$(CC) $(FLAGS) $(INCLUDES) -o example_coordinator $(OBJDIR)/example_coordinator.o $(LIBRARIES)

client: $(OBJS)
	$(CC) $(FLAGS) $(INCLUDES) -o example_client $(OBJDIR)/example_client.o $(LIBRARIES)

clean:
	rm -f $(FILES) $(OBJDIR)/*.o

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(FLAGS) $(INCLUDES) $(LIBS) -c $< -o $@

