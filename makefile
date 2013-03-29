CC = clang++
BOOSTCONNECT_ROOT = ./

CXXFLAGS = -std=c++11
INCDIR = -I$(BOOSTCONNECT_ROOT)include
LIBDIR = -L$(BOOSTCONNECT_ROOT)lib
LIB = -lssl -lcrypto -lpthread -lboostconnect -lssl -lcrypto -lboost_thread -lboost_system

all : target sample
target: ./lib/libboostconnect.a
sample: ./example/server.out ./example/async_client.out ./example/sync_client.out ./example/ssl_client.out

./lib/libboostconnect.a : ./src/boostconnect.cpp
	$(CC) -c $(CXXFLAGS) -o $@ $< $(INCDIR)

./example/server.out : ./example/server/server.cpp
	$(CC) $(CXXFLAGS) -o $@ $< $(INCDIR) $(LIBDIR) $(LIB)

./example/async_client.out : ./example/client/async_client/async_client.cpp
	$(CC) $(CXXFLAGS) -o $@ $< $(INCDIR) $(LIBDIR) $(LIB)

./example/sync_client.out : ./example/client/sync_client/sync_client.cpp
	$(CC) $(CXXFLAGS) -o $@ $< $(INCDIR) $(LIBDIR) $(LIB)

./example/ssl_client.out : ./example/ssl_client/ssl_client.cpp
	$(CC) $(CXXFLAGS) -o $@ $< $(INCDIR) $(LIBDIR) $(LIB)

clean:
	rm ./lib/libboostconnect.a
	rm ./example/server.out
	rm ./example/async_client.out
	rm ./example/sync_client.out
	rm ./example/ssl_client.out
