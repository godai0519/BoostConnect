CC := clang++
BOOSTCONNECT_ROOT := ./

CXXFLAGS = -std=c++11
CXX_DEBUG_FLAGS   = $(CXXFLAGS) -g
CXX_RELEASE_FLAGS = $(CXXFLAGS) -O2

CPPFLAGS = -I$(BOOSTCONNECT_ROOT)include
LDFLAGS  = -L$(BOOSTCONNECT_ROOT)lib -lpthread -lssl -lcrypto -lboost_thread -lboost_system -lbstcon

TARGET = bstcon
DEBUG_TARGET   = ./lib/lib$(TARGET)d.a
RELEASE_TARGET = ./lib/lib$(TARGET).a

SOURSE := src/boostconnect.cpp
HEADER_LIST := $(sort $(dir $(shell find . -name '*.hpp' -or -name '*.ipp')))
SAMPLES  := ./example/server.out ./example/async_client.out ./example/sync_client.out ./example/ssl_client.out

all : debug release
debug :   $(DEBUG_TARGET)   $(SRC_LIST)
release : $(RELEASE_TARGET) $(SRC_LIST)
sample : release $(SAMPLES)

$(DEBUG_TARGET) : $(SOURSE) $(HEADER_LIST)
	$(CC) -c $(CXX_DEBUG_FLAGS) -o $@ $(SOURSE) $(CPPFLAGS)

$(RELEASE_TARGET) : $(SOURCE) $(HEADER_LIST)
	$(CC) -c $(CXX_RELEASE_FLAGS) -o $@ $(SOURSE) $(CPPFLAGS)

./example/server.out : ./example/server/server.cpp
	$(CC) $(CXX_RELEASE_FLAGS) -o $@ $< $(CPPFLAGS) $(LDFLAGS)

./example/async_client.out : ./example/async_client/async_client.cpp
	$(CC) $(CXX_RELEASE_FLAGS) -o $@ $< $(CPPFLAGS) $(LDFLAGS)

./example/sync_client.out : ./example/sync_client/sync_client.cpp
	$(CC) $(CXX_RELEASE_FLAGS) -o $@ $< $(CPPFLAGS) $(LDFLAGS)

./example/ssl_client.out : ./example/ssl_client/ssl_client.cpp
	$(CC) $(CXX_RELEASE_FLAGS) -o $@ $< $(CPPFLAGS) $(LDFLAGS)

clean:
	rm ./lib/libbstcond.a
	rm ./lib/libbstcon.a
	rm ./example/server.out
	rm ./example/async_client.out
	rm ./example/sync_client.out
	rm ./example/ssl_client.out
