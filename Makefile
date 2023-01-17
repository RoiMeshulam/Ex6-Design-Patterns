.PHONY: partA partB partC partD test1 clean
CC=gcc
CPP=g++

partA: Server Client

partB: guard

partC: singleton

partD: pollServer pollClient

test: test.o
	$(CC) test.o -o test -lpthread
test.o: test.c
	$(CC) -c test.c
Server: main1.o
	$(CC) main1.o -o Server -lpthread
main1.o: main1.c
	$(CC) -c main1.c
Client: Client.o
	$(CC) Client.o -o Client -lpthread
Client.o: Client.c
	$(CC) -c Client.c
guard: guard.o
	$(CPP) guard.o -o guard -lpthread
guard.o: guard.cpp
	$(CPP) -c guard.cpp
singleton: singleton.o
	$(CPP) singleton.o -o singleton
singleton.o: singleton.cpp
	$(CPP) -c singleton.cpp
pollServer: pollServer.o
	$(CPP) pollServer.cpp reactor.cpp -o pollServer -pthread -lpthread
pollServer.o: pollServer.cpp
	$(CPP) -c pollServer.cpp
pollClient: pollclient.o
	$(CPP) pollclient.cpp -o pollClient -lpthread
pollclient.o: pollclient.cpp
	$(CPP) -c pollclient.cpp

clean :
	rm -f *.o partA partB partC partD test pollClient pollServer singleton guard Client Server