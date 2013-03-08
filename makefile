
CC = clang++
BOOSTCONNECT_ROOT = ./

CXXFLAGS = -std=c++11
INCDIR = -I$(BOOSTCONNECT_ROOT)include
LIBDIR = -L$(BOOSTCONNECT_ROOT)lib
LIB = -lssl -lcrypto -lpthread -lboostconnect -lssl -lcrypto -lboost_thread -lboost_system

all : target sample
target: ./lib/libboostconnect.a
sample: ./sample/sample.out
run : ./sample/sample.out
	$<

./lib/libboostconnect.a : ./src/boostconnect.cpp
	$(CC) -c $(CXXFLAGS) -o $@ $< $(INCDIR)

./sample/sample.out : ./sample/sample.cpp
	$(CC) $(CXXFLAGS) -o $@ $< $(INCDIR) $(LIBDIR) $(LIB)

clean:
	rm ./lib/libboostconnect.a
	rm ./sample/sample.out
