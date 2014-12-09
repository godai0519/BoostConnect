CC := clang++
BOOSTCONNECT_ROOT := ./

CXXFLAGS = -std=c++11 -DUSE_SSL_BOOSTCONNECT=1
CXX_DEBUG_FLAGS   = $(CXXFLAGS) -g
CXX_RELEASE_FLAGS = $(CXXFLAGS) -O2

CPPFLAGS = -I$(BOOSTCONNECT_ROOT)
LDFLAGS  = -L$(BOOSTCONNECT_ROOT)lib -lpthread -lssl -lcrypto -lboost_thread -lboost_system -lbstcon_client

TARGET = bstcon_client
DEBUG_TARGET   = ./lib/lib$(TARGET)d.a
RELEASE_TARGET = ./lib/lib$(TARGET).a

SOURSE := libs/client/src/client.cpp
HEADER_LIST := $(sort $(dir $(shell find . -name '*.hpp' -or -name '*.ipp')))
SAMPLES  := ./bin/server.out ./bin/async_client.out ./bin/sync_client.out ./bin/ssl_client.out

.PHONY: all clean ./lib

all : ./lib debug release
debug :   ./lib $(DEBUG_TARGET)   $(SRC_LIST)
release : ./lib $(RELEASE_TARGET) $(SRC_LIST)
sample : ./bin release $(SAMPLES)

./bin :
	mkdir -p ./bin

./lib :
	mkdir -p ./lib

$(DEBUG_TARGET) : $(SOURSE) $(HEADER_LIST)
	$(CC) -c $(CXX_DEBUG_FLAGS) -o $@ $(SOURSE) $(CPPFLAGS)

$(RELEASE_TARGET) : $(SOURCE) $(HEADER_LIST)
	$(CC) -c $(CXX_RELEASE_FLAGS) -o $@ $(SOURSE) $(CPPFLAGS)

./bin/server.out : ./example/server/server.cpp
	$(CC) $(CXX_RELEASE_FLAGS) -o $@ $< $(CPPFLAGS) $(LDFLAGS)

./bin/async_client.out : ./example/async_client/async_client.cpp
	$(CC) $(CXX_RELEASE_FLAGS) -o $@ $< $(CPPFLAGS) $(LDFLAGS)

./bin/sync_client.out : ./example/sync_client/sync_client.cpp
	$(CC) $(CXX_RELEASE_FLAGS) -o $@ $< $(CPPFLAGS) $(LDFLAGS)

./bin/ssl_client.out : ./example/ssl_client/ssl_client.cpp
	$(CC) $(CXX_RELEASE_FLAGS) -o $@ $< $(CPPFLAGS) $(LDFLAGS)

clean:
	rm -r ./lib
	rm -r ./bin
