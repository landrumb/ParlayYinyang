//Mohammed, Papa, and Andrew's naive implementation 
//using a simpler point structure while the other one gets debugged
#include "parlay/parallel.h"
#include "parlay/primitives.h"
#include "parlay/sequence.h"
#include "parlay/slice.h"
#include "parlay/random.h"
#include "parlay/internal/get_time.h"
#include "parlay/delayed.h"
#include "parlay/io.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <set>
#include <atomic>
#include <utility>
#include <cmath>
#include <string>

using namespace parlay;

template<typename T> std::pair<T,size_t> anti_overflow_avg_help(const sequence<T> &seq, size_t start, size_t end);


template <typename T> 
struct PPoint {
    parlay::sequence<T> coordinates;
    size_t id;
    bool change_flag = false; //store if a point changed its center in this iteration

    PPoint(size_t d, size_t id) {
        coordinates = sequence<T>(d);
        this->id=id;

    } 

    PPoint() {
        
    }
};
template <typename T>
struct PCenter {
    parlay::sequence<T> coordinates;
    size_t id; //so a cluster knows its own name
    size_t num_members;

    PCenter(){

    }

    PCenter(size_t id, size_t d) {
        this->id = id;
        num_members=0;
        coordinates = sequence<T>(d);

    }

    PCenter(size_t id, size_t num_members, size_t d) {
        this->id = id;
        this->num_members=num_members;
        coordinates=sequence<T>(d);
    }

};


// Define a type for a sequence of points
// using PPoints<T> = parlay::sequence<PPoint<T>>;
// using PCenters = parlay::sequence<PCenter<T>>;

//take your sequences direct from the larger array (which is how we are storing them anyway)
//this version works in parallel
template <typename T> double distanceA_help(const sequence<T>& p1, const sequence<T>& p2,size_t len, size_t dstart, size_t dend) {
    
    if (dend-dstart==0){
        return 0;
    }
    else if (dend-dstart==1) {
        double err = p1[dstart] - p2[dstart]; //avoid the double array access by storing err first then multiplying err*err
        return err*err;
    }
    else {
        double left_sum=0;
        double right_sum=0;
        int mid = (dstart+dend)/2;

        parlay::par_do (
        [&]() {left_sum = distanceA_help(p1,p2,len,dstart,mid);},
        [&]() {right_sum = distanceA_help(p1,p2,len,mid,dend);}
        );

        return left_sum+right_sum;
    }
   
}

//distance from p1 to p2
//p1, p2: points
//returns: the distance (a double)
template <typename T> double distanceA(const PPoint<T>& p1, const PPoint<T>& p2) {
    return distanceA_help(p1.coordinates, p2.coordinates, p1.coordinates.size(),0,p1.coordinates.size());
}

template <typename T> double distanceA(const PCenter<T>& p1, const PPoint<T>& p2) {
    return distanceA_help(p1.coordinates, p2.coordinates, p1.coordinates.size(),0,p1.coordinates.size());
}

template <typename T> double distanceA(const PPoint<T>& p1, const PCenter<T>& p2) {
    return distanceA_help(p1.coordinates, p2.coordinates, p1.coordinates.size(),0,p1.coordinates.size());
}

template <typename T> double distanceA(const PCenter<T>& p1, const PCenter<T>& p2) {
    return distanceA_help(p1.coordinates, p2.coordinates, p1.coordinates.size(),0,p1.coordinates.size());
}

//finds the closest point utilizing the minimum distance
//inspiration from Guy's nice mapping (essential copy)
//@param p: a point 
//@param centers: the centers to which we are going to compare p
//reutrns: the index in centers of the closest center to p
template<typename T> size_t closest_point(const PPoint<T>& p, const sequence<PCenter<T>>& centers) {

    sequence<double> distances = parlay::map(centers, [&] (PCenter<T> centerpoint)  {
    return distanceA(p,centerpoint);
  });

  return std::min_element(distances.begin(),distances.end()) - distances.begin();  

}

//take the avg of seq in a way that doesn't cause overflow
template<typename T> T anti_overflow_avg(const sequence<T>& seq) {
    return anti_overflow_avg_help(seq,0,seq.size()).first;
}

//take the avg of seq in a way that doesn't cause overflow
template<typename T> std::pair<T,size_t> anti_overflow_avg_help(const sequence<T> &seq, size_t start, size_t end) {
    if(start == end - 1){
        return std::make_pair(seq[start],1);
    }   

    size_t mid = (start + end) / 2;
    std::pair<T,size_t> l,r;
    par_do([&](){l = anti_overflow_avg_help(seq,start,mid);}, 
    [&](){r = anti_overflow_avg_help(seq,mid,end);});
    return std::make_pair((l.first*l.second +r.first * r.second)/(l.second + r.second),l.second + r.second);

}

//compute centers calculates the new centers
//returns: a sequence of centers
template<typename T> sequence<PCenter<T>> compute_centers(const sequence<PPoint<T>>& pts, size_t k, size_t d, size_t n) {
    sequence<PCenter<T>> new_centers = sequence<PCenter<T>>::from_function(k, [&] (size_t id) {
        return PCenter<T>(d, id);
    });
    sequence<sequence<size_t>> indices(k);

    for (int i = 0; i < n; i++) {

    //     parlay::parallel_for(0,d,[&] (size_t coord) { //add the value of a point in each coordinate to the new center
    //          new_centers[pts[pt].id] += pts[pt].coordinates[coord];
    //     }, 100); //the last 100 is a granularity parameter? 

    //     new_centers[pts[pt].id].num_members += 1;
        
    // }

    // parlay::parallel_for(0,k*d, [&] (size_t clcoord) {
    //     int cl = clcoord/d;
    //     int coord = clcoord % d;
    //     if (new_centers[cl].num_members !=0) {
    //         new_centers[coord]=new_centers[coord]/new_centers[cl].num_members;
    //     }
    // }, 100);
        indices[pts[i].id].push_back(i);

    }

    
    parallel_for (0, k*d, [&] (size_t icoord){
        size_t i = icoord / d;
        size_t coord = icoord % d;
        new_centers[i].coordinates[coord] = anti_overflow_avg(map(indices[i],[&] (size_t ind) {return pts[ind].coordinates[coord];}  ));

    });

    return new_centers;


}
template <typename T> std::pair<sequence<PCenter<T>>,double> naive_kmeans(parlay::sequence<PPoint<T>>& pts, size_t k, size_t max_iterations, double epsilon){

    parlay::internal::timer timer = parlay::internal::timer();
    timer.start();
    std::cout << "running" << std::endl;

    std::random_device randomize;
    std::mt19937 generate(randomize());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    if(pts.size() == 0){
        return std::make_pair(sequence<PCenter<T>>(2),1.0);
    }
    size_t d = pts[0].coordinates.size();
    size_t n = pts.size();

    // Initialize centers randomly
    sequence<PCenter<T>> centers(k);
    for (int i = 0; i < k; i++) {
        centers[i].coordinates = parlay::sequence<double>(pts[0].coordinates.size(), 0.0);
        for (std::size_t j = 0; j < centers[i].coordinates.size(); j++) {
        centers[i].coordinates[j] = dis(generate);
        }
    }

  int iterations = 0;
  bool converged = false;

  while (!converged && iterations < max_iterations) {
    converged = true;
    iterations++;

    // Assign each point to the closest center
    parlay::parallel_for(0, pts.size(), [&](int i) {
      pts[i].id = closest_point(pts[i], centers);
    });

    // Compute new centers
    sequence<PCenter<T>> new_centers = compute_centers(pts, k, d, n);

    // Check convergence
    double total_diff = 0.0;
    for (int i = 0; i < k; i++) {
      double diff = distanceA(centers[i], new_centers[i]);
      total_diff = std::max(total_diff, diff);
    }

    if (total_diff <= epsilon) {
      converged = true;
    }

    centers = std::move(new_centers);
  }


    return std::make_pair(centers,timer.total_time());

}