cc=g++
server=chatServer
client=chatClient

INCLUDE=-I./lib/include
LDFLAGS=-std=c++11 -lpthread -ljson

.PHONY:all
all:$(server) $(client)

$(server):chatServer.cc
	$(cc) -g -o $@ $^ $(INCLUDE) $(LDFLAGS) 

$(client):chatClient.cc
	$(cc) -g -o $@ $^ $(INCLUDE) $(LDFLAGS)

.PHONY:clean
clean:
	rm -f $(server) $(client)
