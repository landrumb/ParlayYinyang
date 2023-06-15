#include "parlay/parallel.h"
#include "parlay/primitives.h"
#include "parlay/sequence.h"
#include "parlay/slice.h"
#include "parlay/random.h"
#include "parlay/internal/get_time.h"
#include "NSGDist.h"

#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <set>
#include <atomic>
#include <utility>


/* 
    A simple kmeans implementation that uses the naive algorithm
 */
template <typename T>
std::pair<parlay::sequence<center<T>>, double> naive_kmeans(parlay::sequence<point<T>> &v, parlay::sequence<center<T>> &centers, size_t k, size_t max_iterations, Distance& D){
    auto timer = parlay::internal::timer();
    timer.start();

    // initialize the centers by selecting k random points from v
    auto center_indices = parlay::random_permutation(v.size()).slice(0, k);
    // parlay::sequence<center<T>> centers(k);
    for (size_t i = 0; i < k; i++) {
        centers[i] = center<T>(i, parlay::sequence(v[center_indices[i]].coordinates));
    }
    // print progress update with elapsed time
    std::cout << "Initialization: " << timer.next_time() << std::endl;

    // do initial assignment of points to centers
    parlay::parallel_for(0, v.size(), [&](size_t i) {
        v[i].best = 0;
        v[i].ub = D(v[i].coordinates, centers[0].coordinates, v[i].dim);
        for (size_t j = 1; j < k; j++) {
            double dist = D(v[i].coordinates, centers[j].coordinates, v[i].dim);
            if (dist < v[i].ub) {
                v[i].best = j;
                v[i].ub = dist;
            }
        }
        // sketchy atomic parallel insertion
        centers[v[i].best].add_point(v[i].id);
    });

    std::cout << "Initial assignment: " << timer.next_time() << std::endl;


    // update centers
    parlay::parallel_for(0, k, [&](size_t i) {
        parlay::sequence<T> new_center = parlay::reduce(centers[i].points.begin(), centers[i].points.end(), parlay::sequence<T>(v[0].dim), [&](size_t a, size_t b) {
            return parlay::tabulate(v[0].dim, [&](size_t j) {return v[a].coordinates[j] + v[b].coordinates[j];});
        });
        new_center = parlay::map([&](T x) {return x / centers[i].points.size();}, new_center);

        // centers[i].delta += D(centers[i].coordinates, new_center, centers[i].dim);

        // centers[i].coordinates.free() // may be memory leak having this commented
        centers[i].coordinates = new_center;
    });

    std::cout << "Initial center update: " << timer.next_time() << std::endl;

    // do the rest of the iterations
    for (size_t i = 1; i < max_iterations; i++){
        std::atomic<size_t> num_changed(0);

        // do assignment of points to centers
        parlay::parallel_for(0, v.size(), [&](size_t i) {
            size_t old_best = v[i].best;
            v[i].best = 0;
            v[i].ub = D(v[i].coordinates, centers[0].coordinates, v[i].dim);
            for (size_t j = 1; j < k; j++) {
                double dist = D(v[i].coordinates, centers[j].coordinates, v[i].dim);
                if (dist < v[i].ub) {
                    v[i].best = j;
                    v[i].ub = dist;
                }
            }
            if (v[i].best != old_best) {
                num_changed++;
                centers[old_best].remove_point(v[i].id);
                centers[v[i].best].add_point(v[i].id);
            }
        });

        if (num_changed == 0) {
            break;
        }

        // update centers
        // need to update for small integer types
        parlay::parallel_for(0, k, [&](size_t i) {
            parlay::sequence<T> new_center = parlay::reduce(centers[i].points.begin(), centers[i].points.end(), parlay::sequence<T>(v[0].dim), [&](size_t a, size_t b) {
                return parlay::tabulate(v[0].dim, [&](size_t j) {return v[a].coordinates[j] + v[b].coordinates[j];});
            });
            parlay::map([&](T x) {return x / centers[i].points.size();}, new_center);

            // centers[i].delta += D(centers[i].coordinates, new_center, centers[i].dim);

            // centers[i].coordinates.free() // may be memory leak having this commented
            centers[i].coordinates = new_center;
        });

        std::cout << "Iteration " << i << ": " << timer.next_time() << std::endl;
    }

    return std::make_pair(centers, timer.total_time());
};

// template <typename T>
// double kmeans(parlay::sequence)