CXX = gcc
CXXFLAGS = -g -Wall
LDFLAGS =

# We use this to
SOURCES = $(wildcard *.cpp)
HEADERS = $(wildcard *.hpp) $(wildcard *.h)
OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))

# Write the name of your program here
SERVERTARGET = server
CLIENTTARGET = client

.PHONY: all clean

# This "phony" target says we want the target to be built
all: $(SERVERTARGET) $(CLIENTTARGET)

# This "phony" target removes all built files
clean:
	$(RM) server client

# Tells make how to make target out of objects
server: server.c
	$(CXX) -o $@ server.c

client: client.c
	$(CXX) -o $@ client.c

# Tells make how to make objects out of source code
# It also says when we change a header, recompile
# $< is the input, $@ is the output
#%.o: %.cpp $(HEADERS)
#	$(CXX) $(CXXFLAGS) -c $< -o $@
