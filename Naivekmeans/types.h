#ifndef KMEANS_TYPES

#include "../parlay/random.h"
#include "../parlay/parallel.h"
#include "../parlay/primitives.h"
#include "../parlay/sequence.h"
#include "../parlay/slice.h"
#include "../parlay/delayed.h"
#include "../parlay/io.h"
#include "../parlay/internal/get_time.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <set>
#include <atomic>
#include <mutex>

#define KMEANS_TYPES

/* 
    It's desirable that the point struct not handle much clustering-implementation-specific logic so parsing code can be easily reused
 */


struct stats {
    parlay::sequence<size_t> dist_calls;
    parlay::sequence<double> SSE;
    parlay::sequence<double> runtimes;
    double total_runtime;
};

template <typename T>
struct point {

    parlay::slice<T*, T*> coordinates; // the coordinates of the point
    size_t best; // the index of the best center for the point

   
    point() : best(-1), coordinates(nullptr, nullptr) {
    }


};

template <typename T>
struct center {
    size_t id; // a unique (hopefully) identifier for the center
    parlay::sequence<T> coordinates; // the pointer to coordinates of the center
   
    center(size_t id, parlay::sequence<T> coordinates) : id(id) {
      
        this->coordinates = coordinates;
    }

    center() : id(-1) {
       
    }

};

//useful helper functions

template <typename T> void print_point(const point<T>& p) {
  std::cout << "Po: " << p.best << std::endl;
  for (T* i = p.coordinates.begin(); i != p.coordinates.end(); i = std::next(i) ) {
    std::cout << *i << " ";
  }
  std::cout << std::endl;
}

//for uint8, we need to static cast to int for the output to not be gibberish (otherwise we're outputting an unsigned char)
template <> void print_point<uint8_t>(const point<uint8_t>& p) {
    std::cout<< "uint8 specialized print\n";
    for (uint8_t* i = p.coordinates.begin(); i != p.coordinates.end(); i = std::next(i) ) {
        std::cout << static_cast<int> (*i) << " ";
    }
  std::cout << std::endl;

}

template <typename T> void print_center(const center<T>& c) {
  std::cout << "ID" << c.id << std::endl;
  std::cout <<"COORDS ";
  for (int i = 0; i < c.coordinates.size(); i++) {
    std::cout << c.coordinates[i] << " ";
  }

  std::cout << std::endl;
  
}

template <> void print_center<uint8_t>(const center<uint8_t>& c) {
  std::cout << "ID" << c.id << std::endl;
  std::cout <<"COORDS ";
  for (int i = 0; i < c.coordinates.size(); i++) {
    std::cout << static_cast<int>(c.coordinates[i]) << " ";
  }

  std::cout << std::endl;
  
}

parlay::sequence<size_t> randseq(size_t n, size_t k){
    assert(n > k);
    std::random_device randomizer;
    std::mt19937 generate(randomizer());
   // std::uniform_int_distribution<size_t> dis(1, n);

    parlay::sequence<size_t> random_numbers(k);
    size_t bucket = n / k;

    parlay::parallel_for(0, k, [&](size_t i){
         std::uniform_int_distribution<size_t> dis(bucket * i, bucket * (i+1) - 1);
        random_numbers[i] = dis(generate);
    });
  

  return random_numbers;
}


template<typename T> void print_seq(parlay::sequence<T> seq) {
    for (int i = 0; i < seq.size(); i++) {
        std::cout << seq[i] << " ";
    }
    std::cout << std::endl;
}

template<> void print_seq <uint8_t> (parlay::sequence<uint8_t> seq) {
     for (int i = 0; i < seq.size(); i++) {
        std::cout << static_cast<int>(seq[i]) << " ";
    }
    std::cout << std::endl;

}

#endif