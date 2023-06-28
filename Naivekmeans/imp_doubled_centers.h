//this version of our implementation does not require a dataset of doubles, casts them when needed instead

#ifndef IMP_DC
#define IMP_DC

//print out a ton of debug info if true

bool DEBUG_DC = false;

#include "types.h"
#include "imp_with_euc_dist.h"
#include "center_creation.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <set>
#include <atomic>
#include <mutex>

using namespace parlay;


//put the coordinates of p onto the stack for the calculation
template<typename T> size_t closest_point_convert(const point<T>& p, sequence<center<double>>& centers) {
    if constexpr(std::is_same<T,double>() == true) {
        //run the normal closest elt
        return closest_point_euc_dist(p,centers);
    }
    else {
        assert(centers.size() > 0);
        assert(p.coordinates.size() == centers[0].coordinates.size());
        double buf[p.coordinates.size()];
        T* it = p.coordinates.begin();
        for (int i = 0; i < p.coordinates.size(); i++) {
            buf[i]=*it;
            it += 1; //add 1 for next?
        }
        auto distances = parlay::delayed::map(centers, [&](const center<double>& q) {
            return euclidean_squared(buf, q.coordinates);
        });
        return min_element(distances) - distances.begin();

    }

}


//compute centers calculates the new centers
//returns: a sequence of centers
template<typename T> sequence<center<double>> compute_centers_double(const sequence<point<T>>& pts, const sequence<center<double>>& centers, size_t k, size_t d, size_t n) {

    if constexpr(std::is_same<T,double>() == true) {
        //run the normal compute centers
        return compute_centers_ec(pts,centers,k,d,n);
    }

    sequence<sequence<size_t>> indices(k);
    
    sequence<center<double>> new_centers(k);
    for (int i = 0; i < k; i++) {
        new_centers[i].id = i;
        new_centers[i].coordinates=sequence<double>(d,4);
    }


    for (int i = 0; i < n; i++) {
            
        indices[pts[i].best].push_back(i); //it's called best not id!!!

    }
    if (DEBUG_DC) {
        std::cout << "Debugging: printing out center counts:\n";
        for (int i = 0; i < k; i++) {
        std::cout << indices[i].size() << std::endl;
        }

    }
    
    
    parlay::parallel_for (0, k*d, [&] (size_t icoord){
        size_t i = icoord / d;
        size_t coord = icoord % d;

      
        //if there are no values in a certain center, just keep the center where it is
        if (indices[i].size() > 0) { //anti_overflow_avg or reduce?? //note the static cast to double here, because points are whatever
            new_centers[i].coordinates[coord] = reduce(map(indices[i],[&] (size_t ind) {return static_cast<double>(pts[ind].coordinates[coord]);})) / indices[i].size(); //normal averaging now

        }
        else { 
            new_centers[i].coordinates[coord] = centers[i].coordinates[coord];
        }


    });


    return new_centers;


}





template <typename T> std::pair<sequence<center<double>>,double> kmeans_double_center(parlay::sequence<point<T>>& pts, parlay::sequence<center<double>>& centers, size_t k, size_t max_iterations, double epsilon){

    std::cout << "running dc" << std::endl;

    parlay::internal::timer timer = parlay::internal::timer();
    timer.start();
    

     if(pts.size() == 0){ //default case
        return std::make_pair(sequence<center<double>>(2),1.0);
    }

    size_t d = pts[0].coordinates.size();
    size_t n = pts.size();


    if constexpr(std::is_same<T,double>() == true) {
        //run the normal compute centers
        return kmeans_euc_dist(pts,centers,k,d,n);
    }



  int iterations = 0;

  double total_diff = 0;

  while (iterations < max_iterations) {

    if (DEBUG_DC) {
         std::cout << "centers: " << iterations << std::endl;
    for (int i = 0; i < k; i++) {
       print_center(centers[i]);        
    }

    }
    

   
    std::cout << "iter" << iterations << std::endl;
    iterations++;

    if (DEBUG_DC) {
        std::cout << "center printing" << std::endl;
        for (int i = 0; i < k; i++) {
            print_center(centers[i]);
        }
        std::cout << std::endl << std::endl;

    }
     

    // Assign each point to the closest center
    parlay::parallel_for(0, pts.size(), [&](size_t i) {
      pts[i].best = closest_point_convert(pts[i], centers);
    });

    std::cout << "past closest pt\n";

    // Compute new centers
    sequence<center<double>> new_centers = compute_centers_double(pts, centers, k, d, n);

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
    if (DEBUG_DC) {
         std::cout << "center printing" << std::endl;
  for (int i = 0; i < k; i++) {
    print_center(centers[i]);
  }
  std::cout << "Error" << total_diff << std::endl;
  std::cout << std::endl << std::endl;

    }
  

  return std::make_pair(centers,timer.total_time());

    //return std::make_pair(centers,timer.total_time());

}

#endif //IMP_DC