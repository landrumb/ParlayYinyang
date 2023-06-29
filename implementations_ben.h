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
#include <type_traits>

/* 
    A simple kmeans implementation that uses the naive algorithm
 */
// template <typename T>
// std::pair<parlay::sequence<center<T>>, double> naive_kmeans(parlay::sequence<point<T>> &v, parlay::sequence<center<T>> &centers, size_t k, size_t max_iterations, Distance D){
//     auto timer = parlay::internal::timer();
//     timer.start();

//     size_t dim = v[0].dim;

//     // initialize the centers by selecting k random points from v
//     auto center_indices = parlay::random_permutation(v.size()).cut(0, k);
//     // parlay::sequence<center<T>> centers(k);
//     for (size_t i = 0; i < k; i++) {
//         auto coordinates = parlay::tabulate(dim, [&](size_t j) {return v[center_indices[i]].coordinates[j];});
//         centers[i].coordinates = coordinates;
//         // centers[i].coordinates_slice = parlay::make_slice(coordinates);
//         centers[i].id = i;
//         centers[i].dim = dim;
//     }
//     // print progress update with elapsed time
//     std::cout << "Initialization: " << timer.next_time() << std::endl;

//     // do initial assignment of points to centers
//     parlay::parallel_for(0, v.size(), [&](size_t i) {
//         v[i].best = 0;
//         v[i].ub = D(v[i].coordinates, parlay::make_slice(centers[0].coordinates), dim);
//         for (size_t j = 1; j < k; j++) {
//             double dist = D(v[i].coordinates, parlay::make_slice(centers[j].coordinates), dim);
//             if (dist < v[i].ub) {
//                 v[i].best = j;
//                 v[i].ub = dist;
//             }
//         }
//         // parallel insertion with mutex
//         centers[v[i].best].add_point(v[i].id);
//     });

    // std::cout << "Initial assignment: " << timer.next_time() << std::endl;


    // // update centers
    // parlay::parallel_for(0, k, [&](size_t i) {
    //     parlay::sequence<T> new_center = parlay::reduce(centers[i].points.begin(), centers[i].points.end(), parlay::sequence<T>(v[0]->dim), [&](size_t a, size_t b) {
    //         return parlay::tabulate(v[0].dim, [&](size_t j) {return v[a].coordinates[j] + v[b].coordinates[j];});
    //     });
    //     new_center = parlay::map([&](T x) {return x / centers[i].points.size();}, new_center);

    //     // centers[i].delta += D(centers[i].coordinates, new_center, centers[i].dim);

    //     // centers[i].coordinates.free() // may be memory leak having this commented
    //     for (size_t j = 0; j < v[0].dim; j++) {
    //         centers[i].coordinates[j] = new_center[j];
    //     }
    // });

    // std::cout << "Initial center update: " << timer.next_time() << std::endl;

    // // do the rest of the iterations
    // for (size_t i = 1; i < max_iterations; i++){
    //     std::atomic<size_t> num_changed(0);

    //     // do assignment of points to centers
    //     parlay::parallel_for(0, v.size(), [&](size_t i) {
    //         size_t old_best = v[i].best;
    //         v[i].best = 0;
    //         v[i].ub = D(v[i].coordinates, centers[0].coordinates_slice, v[i]->dim);
    //         for (size_t j = 1; j < k; j++) {
    //             double dist = D(v[i].coordinates, centers[j].coordinates_slice, v[i]->dim);
    //             if (dist < v[i].ub) {
    //                 v[i].best = j;
    //                 v[i].ub = dist;
    //             }
    //         }
    //         if (v[i].best != old_best) {
    //             num_changed++;
    //             centers[old_best].remove_point(v[i].id);
    //             centers[v[i].best].add_point(v[i].id);
    //         }
    //     });

    //     if (num_changed == 0) {
    //         break;
    //     }

    //     // update centers
    //     // need to update for small integer types
    //     parlay::parallel_for(0, k, [&](size_t i) {
    //         parlay::sequence<T> new_center = parlay::reduce(centers[i].points.begin(), centers[i].points.end(), parlay::sequence<T>(v[0]->dim), [&](size_t a, size_t b) {
    //             return parlay::tabulate(v[0]->dim, [&](size_t j) {return v[a].coordinates[j] + v[b].coordinates[j];});
    //         });
    //         parlay::map([&](T x) {return x / centers[i].points.size();}, new_center);

    //         // centers[i].delta += D(centers[i].coordinates, new_center, centers[i].dim);

    //         // centers[i].coordinates.free() // may be memory leak having this commented
    //         // centers[i].coordinates = new_center;
    //         for (size_t j = 0; j < v[0].dim; j++) {
    //             centers[i].coordinates[j] = new_center[j];
    //         }
    //     });

    //     std::cout << "Iteration " << i << ": " << timer.next_time() << std::endl;
    // }

//     return std::make_pair(centers, timer.total_time());
// };

template <typename T>
double kmeans(parlay::sequence<point<T>> &v, parlay::sequence<center<T>> &centers, size_t k, size_t max_iterations, Distance D){
    auto timer = parlay::internal::timer();
    timer.start();

    size_t dim = v[0].coordinates.size();
    size_t n = v.size();
    
    // initialize the centers by selecting k random points from v
    auto center_indices = parlay::random_permutation(n).cut(0, k);
    for (size_t i = 0; i < k; i++) {
        for (size_t j = 0; j < dim; j++) {
            // std::cout << j << std::endl;
            centers[i].coordinates[j] = v[center_indices[i]].coordinates[j];
        }
        centers[i].id = i;
        centers[i].dim = dim;
    }

    // initial assignment of points to centers
    parlay::parallel_for(0, n, [&](size_t i) {
        v[i].best = 0;
        v[i].ub = D.distance(v[i].coordinates.begin(), centers[0].coordinates.begin(), dim);
        for (size_t j = 1; j < k; j++) {
            double dist = D.distance(v[i].coordinates.begin(), centers[j].coordinates.begin(), dim);
            if (dist < v[i].ub) {
                v[i].best = j;
                v[i].ub = dist;
            }
        }
        centers[v[i].best].add_point(i);
    });

    std::cout << "Initial assignment: " << timer.next_time() << std::endl;

    // initial center update
    parlay::parallel_for(0, k, [&](size_t i) {
        parlay::sequence<double> new_center(dim);
        size_t num_points = centers[i].points.size();
        for (auto it = centers[i].points.begin(); it != centers[i].points.end(); it++) {
            for (size_t j = 0; j < dim; j++) {
                new_center[j] += v[*it].coordinates[j];
            }
        }
        for (size_t j = 0; j < dim; j++) {
            centers[i].coordinates[j] = static_cast<T>(std::round(new_center[j] / (double) num_points));
        }
    });

    std::cout << "Initial center update: " << timer.next_time() << std::endl;

    for (size_t iteration=1; iteration <= max_iterations; iteration++){
        // update assignments
        std::atomic<size_t> num_changed(0);
        parlay::parallel_for(0, n, [&](size_t i) {
            size_t old_best = v[i].best;
            for (size_t j = 1; j < k; j++) {
                double dist = D.distance(v[i].coordinates.begin(), centers[j].coordinates.begin(), dim);
                if (dist < v[i].ub) {
                    num_changed++;
                    v[i].best = j;
                    v[i].ub = dist;
                }
            }
            if (v[i].best != old_best) {
                num_changed++;
                centers[old_best].remove_point(i);
                centers[v[i].best].add_point(i);
            }
        });

        if (num_changed == 0) {
            std::cout << "converged!" << std::endl;
            break;
        }

        // update centers
        parlay::parallel_for(0, k, [&](size_t i) {
            parlay::sequence<double> new_center(dim);
            size_t num_points = centers[i].points.size();
            for (auto it = centers[i].points.begin(); it != centers[i].points.end(); it++) {
                for (size_t j = 0; j < dim; j++) {
                    new_center[j] += v[*it].coordinates[j];
                }
            }
            for (size_t j = 0; j < dim; j++) {
                centers[i].coordinates[j] = static_cast<T>(std::round(new_center[j] / (double) num_points));
            }
        });

        std::cout << "Iteration " << iteration << ": " << timer.next_time() << std::endl;
    }

    // std::cout << "Total time: " << timer.total_time() << std::endl;

    return timer.total_time();
}