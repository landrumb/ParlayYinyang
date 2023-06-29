
#Not sure what this does, artifact from ParlayANN
ifeq (, $(shell which jemalloc-config))
JEMALLOC =
else
JEMALLOCLD = $(shell jemalloc-config --libdir)
JEMALLOC = -L$(JEMALLOCLD) -ljemalloc 
endif

CCFLAGS = -mcx16 -O3 -std=c++17 -march=native -DNDEBUG -I .

ANDREW_CLFLAGS = -ldl $(JEMALLOC) -lstdc++ #notice the -stdc++
CLFLAGS = -ldl $(JEMALLOC) 

PBBFLAGS = -DHOMEGROWN -pthread

CC = g++ 
ANDREW_CC = gcc-13

CFLAGS = $(PBBFLAGS) $(CCFLAGS)
LFLAGS = $(PBBFLAGS) $(CLFLAGS)
ANDREW_LFLAGS = $(PBBFLAGS) $(ANDREW_CLFLAGS)

ANDREW_FLAGS = $(ANDREW_CC) $(CFLAGS) $(ANDREW_LFLAGS)

INCLUDE = -I /parlay/ -I .

all: distcheck

distcheck: distanceBenchmark.cc NSGDist.h types.h
	$(CC) $(CFLAGS) $(INCLUDE) $(LFLAGS)  -o distcheck distanceBenchmark.cc 

dist_test: distanceTest.cc NSGDist.h
	$(CC) $(CFLAGS) $(INCLUDE) $(LFLAGS) -o dist_test distanceTest.cc

andrew_distcheck: distanceBenchmark.cc NSGDist.h types.h
	$(ANDREW_CC) $(CFLAGS) $(INCLUDE) $(ANDREW_LFLAGS)  -o distcheck distanceBenchmark.cc 

andrew_dist_test: distanceTest.cc NSGDist.h
	$(ANDREW_CC) $(CFLAGS) $(INCLUDE) $(ANDREW_LFLAGS) -o dist_test distanceTest.cc

a_dist_check2: distanceBenchmark_withComments.cc NSGDist.h
	$(ANDREW_CC) $(CFLAGS) $(INCLUDE) $(ANDREW_LFLAGS)  -o dist_check2 distanceBenchmark_withComments.cc 



clean:
	rm -f distcheck dist_test andrew_dist_test andrew_distcheck dist_check2