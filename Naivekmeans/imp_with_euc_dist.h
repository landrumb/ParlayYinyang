//This version of kmeans (building off of naivekmeans in impshared_4) will use the much faster euclidean_distance function,
//instead of our distanceA

//Starting a new file because impshared_3 a bit messy
//Writing a naive kmeans implementation that corresponds to Ben's typing:
//namely, the point and center structs
//note that the Distance function isn't finish so using a homemade distance function, can add the distance formatting later
//Andrew, Mohammed, Papa

//print out a ton of debug info if true

#ifndef IMP_ED
#define IMP_ED

bool DEBUG_ED = false;

#include "types.h"
#include "center_creation.h"


#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <set>
#include <atomic>
#include <mutex>

using namespace parlay;


//finds the closest point utilizing the minimum distance
//inspiration from Guy's nice mapping (essential copy)
//@param p: a point 
//@param centers: the centers to which we are going to compare p
//reutrns: the index in centers of the closest center to p
//note: removed const from centers
template<typename T> size_t closest_point_euc_dist(const point<T>& p, sequence<center<T>>& centers) {

    assert(centers.size() > 0); //centers must be nonempty
    //std::cout << p.coordinates.size() << " " << centers[0].dim << std::endl;
    assert(p.coordinates.size()==centers[0].coordinates.size()); //points and centers should have the same # of dimensions

    sequence<double> distances(centers.size());
    for (int i = 0; i < distances.size(); i++) {
        distances[i] = euclidean_squared(p.coordinates,make_slice(centers[i].coordinates));
    }

  return std::min_element(distances.begin(),distances.end()) - distances.begin();  

}

//compute centers calculates the new centers
//returns: a sequence of centers
template<typename T> sequence<center<T>> compute_centers_ec(const sequence<point<T>>& pts, const sequence<center<T>>& centers, size_t k, size_t d, size_t n) {

    sequence<sequence<size_t>> indices(k);
    
    sequence<center<T>> new_centers(k);
    for (int i = 0; i < k; i++) {
        new_centers[i].id = i;
        new_centers[i].coordinates=sequence<T>(d,4);
    }


    for (int i = 0; i < n; i++) {
            
        indices[pts[i].best].push_back(i); //it's called best not id!!!

    }
    if (DEBUG_ED) {
        std::cout << "Debugging: printing out center counts:\n";
        for (int i = 0; i < k; i++) {
        std::cout << indices[i].size() << std::endl;
        }

    }
    
    
    parlay::parallel_for (0, k*d, [&] (size_t icoord){
        size_t i = icoord / d;
        size_t coord = icoord % d;

        //std::cout<<"icoord " << icoord << "i : " << i << "coord: " << coord << std::endl;
        //new_centers[i].coordinates[coord] = anti_overflow_avg(map(new_centers[i].points,[&] (size_t ind) {return pts[ind].coordinates[coord];}  ));

        //if there are no values in a certain center, just keep the center where it is
        if (indices[i].size() > 0) { //anti_overflow_avg or reduce??
            new_centers[i].coordinates[coord] = reduce(map(indices[i],[&] (size_t ind) {return pts[ind].coordinates[coord];})) / indices[i].size(); //normal averaging now

        }
        else { 
            new_centers[i].coordinates[coord] = centers[i].coordinates[coord];
        }


    });


    return new_centers;


}


template <typename T> double kmeans_euc_dist(parlay::sequence<point<T>>& pts, parlay::sequence<center<T>>& centers, size_t k, size_t max_iterations, double epsilon){

    parlay::internal::timer timer = parlay::internal::timer();
    timer.start();

    std::cout << "running" << std::endl;

     if(pts.size() == 0){ //default case
        return -1;
    }

    size_t d = pts[0].coordinates.size();
    size_t n = pts.size();

    int iterations = 0;

    double total_diff = 0;

    while (iterations < max_iterations) {

        if (DEBUG_ED) {
            std::cout << "centers: " << iterations << std::endl;
        for (int i = 0; i < k; i++) {
        print_center(centers[i]);        
        }

        }
    
    std::cout << "iter" << iterations << std::endl;
    iterations++;

    // Assign each point to the closest center
    parlay::parallel_for(0, pts.size(), [&](size_t i) {
      pts[i].best = closest_point_euc_dist(pts[i], centers);
    });

    std::cout << "past closest pt\n";

    // Compute new centers
    sequence<center<double>> new_centers = compute_centers_ec(pts, centers, k, d, n);

    // Check convergence
    total_diff = 0.0;
    for (int i = 0; i < k; i++) {
      double diff = euclidean_squared(centers[i].coordinates, new_centers[i].coordinates);
      total_diff += diff;
    }

    centers = std::move(new_centers);

    std::cout << "difs " << total_diff << " " << epsilon << std::endl;

    if (total_diff <= epsilon) {
      break;
    }
    
  }
    if (DEBUG_ED) {
         std::cout << "center printing" << std::endl;
  for (int i = 0; i < k; i++) {
    print_center(centers[i]);
  }
  std::cout << "Error" << total_diff << std::endl;
  std::cout << std::endl << std::endl;

    }
  
    return timer.total_time();

}

#endif //IMP_ED