include parallelDefsANN

INCLUDE = -I /parlay/ -I .

all: kmeans

kmeans: kmeans.cpp impshared_4.h 
	$(CC) $(CFLAGS) $(INCLUDE) -o kmeans kmeans.cpp $(LFLAGS)

clean:
	rm -f kmeans