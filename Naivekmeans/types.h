#ifndef KMEANS_TYPES

#include "../parlay/random.h"
#include "../parlay/parallel.h"
#include "../parlay/primitives.h"
#include "../parlay/sequence.h"
#include "../parlay/slice.h"
#include "../parlay/delayed.h"
#include "../parlay/io.h"
#include "../parlay/internal/get_time.h"


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

#endif