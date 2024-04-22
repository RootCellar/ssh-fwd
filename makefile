CXX = gcc
CXXFLAGS = -g -Wall
LDFLAGS =

# Program names
SERVERTARGET = server
CLIENTTARGET = client

.PHONY: all clean

# Build client and server by default
all: $(SERVERTARGET) $(CLIENTTARGET)

# Cleans targets

clean:
	$(RM) server client

# Targets

server: server.c
	$(CXX) -o $@ server.c

client: client.c $(HEADERS)
	$(CXX) -o $@ client.c
