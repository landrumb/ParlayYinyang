#ifndef KMEANS_TYPES

#include "parlay/parallel.h"
#include "parlay/primitives.h"
#include "parlay/sequence.h"
#include "parlay/slice.h"
#include "parlay/random.h"

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
template <typename T>
struct point {
    size_t id; // a unique (hopefully) identifier for the point
    // size_t dim; // the dimension of the point
    parlay::slice<T*, T*> coordinates; // the coordinates of the point
    double ub; // the upper bound of the point
    size_t best; // the index of the best center for the point

    // point(size_t id, size_t dim, T* vector) : id(id), dim(dim), coordinates(vector, vector + dim) {
    //     this->ub = std::numeric_limits<double>::max();
    //     this->best = -1;
    // }

    point() : id(-1), coordinates(nullptr, nullptr) {
        this->ub = std::numeric_limits<double>::max();
        this->best = -1;
    }


};

template <typename T>
struct center {
    size_t id; // a unique (hopefully) identifier for the center
    size_t dim; // the dimension of the center
    parlay::sequence<T> coordinates; // the pointer to coordinates of the center
    // parlay::slice<T*, T*> coordinates_slice; // a slice of the coordinates of the center
    double delta; // the delta of the center
    std::set<size_t> points; // the indices of the points in the center
    std::mutex points_mutex; // a mutex to lock the points set
    
    center(size_t id, size_t dim, parlay::sequence<T> coordinates) : id(id), dim(dim){
        this->delta = 0;
        this->points = std::set<size_t>();
        this->coordinates = coordinates;
        // this->coordinates_slice = parlay::make_slice(coordinates);
    }

    center() : id(-1), dim(-1) {
        this->delta = 0;
        // this->points = std::set<size_t>();
    }

    void add_point(size_t point_id) {
        this->points_mutex.lock();
        this->points.insert(point_id);
        this->points_mutex.unlock();
    }

    void remove_point(size_t point_id) {
        this->points_mutex.lock();
        this->points.erase(point_id);
        this->points_mutex.unlock();
    }

    void clear_points() {
        this->points_mutex.lock();
        this->points.clear();
        this->points_mutex.unlock();
    }
};

template <typename T>
struct group {
    size_t id; // a unique (hopefully) identifier for the group
    parlay::sequence<center<T>*> centers; // pointers to the centers in the group

};

#endif