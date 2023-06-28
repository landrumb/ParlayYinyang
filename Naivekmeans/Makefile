
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

kmeans_stats: kmeans_stats.cpp impshared_4_stats.h 
	$(CC) $(CFLAGS) $(INCLUDE) $(LFLAGS)  -o kmeans kmeans_stats.cpp  

a_kmeans4: kmeans.cpp impshared_4.h kmeans_guy.h types.h

	$(ANDREW_FLAGS)  -o kmeans4 kmeans.cpp  

# a_kmeans5: kmeans.cpp impshared_5.h types.h NSGDist.h
# 	$(ANDREW_FLAGS)  -o kmeans5 kmeans_improved.cpp

kmeans_integrated: kmeans_integrated.cpp imp_with_euc_dist.h imp_doubled_centers.h impshared_4.h kmeans_guy.h types.h
	$(ANDREW_FLAGS)  -o kmeans_integrated kmeans_integrated.cpp  

clean:
	rm -f kmeans kmeans4 kmeans5 kmeans_integrated