CXX = gcc
CXXFLAGS = -g -Wall
LDFLAGS =

SOURCES = $(wildcard *.cpp)
HEADERS = $(wildcard *.hpp) $(wildcard *.h)
OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))

SERVERTARGET = server
CLIENTTARGET = client

.PHONY: all clean

all: $(SERVERTARGET) $(CLIENTTARGET)

clean:
	$(RM) server client

server: server.c $(HEADERS)
	$(CXX) -o $@ server.c

client: client.c $(HEADERS)
	$(CXX) -o $@ client.c
