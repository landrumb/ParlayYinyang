include parallelDefsANN

INCLUDE = -I /parlay/ -I .

all: kmeans

kmeans: kmeans.cpp implementations_ben.h 
	$(CC) $(CFLAGS) $(INCLUDE) -o kmeans kmeans.cpp $(LFLAGS)

clean:
	rm -f kmeans