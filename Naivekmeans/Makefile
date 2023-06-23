
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

all: kmeans

kmeans: kmeans.cpp impshared_4.h 
	$(CC) $(CFLAGS) $(INCLUDE) $(LFLAGS)  -o kmeans kmeans.cpp  

andrew_kmeans: kmeans.cpp impshared_4.h
	$(ANDREW_FLAGS)  -o kmeans kmeans.cpp  


clean:
	rm -f kmeans