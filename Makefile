include parallelDefsANN

INCLUDE = -I /parlay/ -I .

all: kmeans

kmeans: kmeans.cpp implementations_ben.h 
	$(CC) $(CFLAGS) $(INCLUDE) -o kmeans kmeans.cpp $(LFLAGS)

accumulator_test: accumulator_test.cpp accumulator.h
	$(CC) $(CFLAGS) $(INCLUDE) -o accumulator_test accumulator_test.cpp $(LFLAGS)

clean:
	rm -f kmeans