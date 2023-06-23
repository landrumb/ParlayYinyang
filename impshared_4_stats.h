//Starting a new file because impshared_3 a bit messy
//Writing a naive kmeans implementation that corresponds to Ben's typing:
//namely, the point and center structs
//note that the Distance function isn't finish so using a homemade distance function, can add the distance formatting later
//Andrew, Mohammed, Papa

#include "types.h"


#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <set>
#include <atomic>
#include <mutex>

using namespace parlay;


//function signatures
template<typename T> std::pair<T,size_t> anti_overflow_avg_help(const sequence<T> &seq, size_t start, size_t end);
template <typename T> std::pair<sequence<center<T>>,double> naive_kmeans(parlay::sequence<point<T>>& pts, size_t k, size_t max_iterations, double epsilon);


sequence<size_t> randseq(size_t n, size_t k){
    assert(n > k);
    std::random_device randomizer;
    std::mt19937 generate(randomizer());
   // std::uniform_int_distribution<size_t> dis(1, n);

    sequence<size_t> random_numbers(k);
    size_t bucket = n / k;

    parallel_for(0, k, [&](size_t i){
         std::uniform_int_distribution<size_t> dis(bucket * i, bucket * (i+1) - 1);
        random_numbers[i] = dis(generate);
    });
  

  return random_numbers;
}

template <typename T> void print_point(const point<T>& p) {
  std::cout << "Po: " << p.id << " " << p.best << std::endl;
  for (T* i = p.coordinates.begin(); i != p.coordinates.end(); i = std::next(i) ) {
    std::cout << *i << " ";
  }
  std::cout << std::endl;
}

template <typename T> void print_center(const center<T>& c) {
  std::cout << "ID" << c.id << std::endl;
  std::cout <<"COORDS ";
  for (int i = 0; i < c.coordinates.size(); i++) {
    std::cout << c.coordinates[i] << " ";
  }
  std::cout<<"Members: ";
  std::cout << std::endl; //lol lifehack change type based on error msg
  for (std::set<size_t>::const_iterator i = c.points.begin(); i != c.points.end(); i=std::next(i)) {
    std::cout << *i << " ";


  }
  std::cout << std::endl;
  
}

//hmm have to pass by value? why? (double vs const double error)
//version of distance that works with slices
template <typename T> double distanceA(slice<T*,T*> p1, slice<T*,T*> p2,size_t dstart, size_t dend) {
    
    assert(dstart <= dend); //the end should never be less than the start

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
        [&]() {left_sum = distanceA(p1,p2,dstart,mid);},
        [&]() {right_sum = distanceA(p1,p2,mid,dend);}
        );


        return left_sum+right_sum;
    }
   
}

//version of distance that works with slices
template <typename T> double distanceA(sequence<T> p1, sequence<T> p2,size_t dstart, size_t dend) {
    
    assert(dstart <= dend); //the end should never be less than the start

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
        [&]() {left_sum = distanceA(p1,p2,dstart,mid);},
        [&]() {right_sum = distanceA(p1,p2,mid,dend);}
        );


        return left_sum+right_sum;
    }
   
}

//finds the closest point utilizing the minimum distance
//inspiration from Guy's nice mapping (essential copy)
//@param p: a point 
//@param centers: the centers to which we are going to compare p
//reutrns: the index in centers of the closest center to p
//note: removed const from centers

template<typename T> 
pair<size_t, size_t> closest_point_stats(const point<T>& p, sequence<center<T>>& centers) {

    assert(centers.size() > 0); //centers must be nonempty
    //std::cout << p.coordinates.size() << " " << centers[0].dim << std::endl;
    assert(p.coordinates.size()==centers[0].coordinates.size()); //points and centers should have the same # of dimensions
    size_t calls = 0;
    sequence<double> distances(centers.size());
    for (int i = 0; i < distances.size(); i++) {
        distances[i] = distanceA(p.coordinates,make_slice(centers[i].coordinates),0,centers[i].coordinates.size());
        calls++;
    }

  return make_pair(std::min_element(distances.begin(),distances.end()) - distances.begin(), calls);  

}



//take the avg of seq in a way that doesn't cause overflow
template<typename T> T anti_overflow_avg(const sequence<T>& seq) {
    return anti_overflow_avg_help(seq,0,seq.size()).first;
}


//take the avg of seq in a way that doesn't cause overflow
template<typename T> std::pair<T,size_t> anti_overflow_avg_help(const sequence<T> &seq, size_t start, size_t end) {
    if(start == end - 1){ // if end-start < Bound do sequentially
        return std::make_pair(seq[start],1);
    }   

    size_t mid = (start + end) / 2;
    std::pair<T,size_t> l,r;
    parlay::par_do(
    [&](){l = anti_overflow_avg_help(seq,start,mid);}, 
    [&](){r = anti_overflow_avg_help(seq,mid,end);}
    );


    return std::make_pair((l.first*l.second +r.first * r.second)/(l.second + r.second),l.second + r.second);

}


//compute centers calculates the new centers
//returns: a sequence of centers
template<typename T> sequence<center<T>> compute_centers(const sequence<point<T>>& pts, const sequence<center<T>>& centers, size_t k, size_t d, size_t n) {

    sequence<sequence<size_t>> indices(k);
    
    sequence<center<T>> new_centers(k);
    for (int i = 0; i < k; i++) {
        new_centers[i].id = i;
        new_centers[i].coordinates=sequence<T>(d,4);
    }


    for (int i = 0; i < n; i++) {
            
        indices[pts[i].best].push_back(i); //it's called best not id!!!

    }
    
    parlay::parallel_for (0, k*d, [&] (size_t icoord){
        size_t i = icoord / d;
        size_t coord = icoord % d;

        //std::cout<<"icoord " << icoord << "i : " << i << "coord: " << coord << std::endl;
        //new_centers[i].coordinates[coord] = anti_overflow_avg(map(new_centers[i].points,[&] (size_t ind) {return pts[ind].coordinates[coord];}  ));

        //if there are no values in a certain center, just keep the center where it is
        if (indices[i].size() > 0) {
            new_centers[i].coordinates[coord] = anti_overflow_avg(map(indices[i],[&] (size_t ind) {return pts[ind].coordinates[coord];}  ));

        }
        else { 
            new_centers[i].coordinates[coord] = centers[i].coordinates[coord];
        }


    });


    return new_centers;


}
        //new_centers[i].coordinates[coord] = anti_overflow_avg(map(indices[pts[i].best],[&] (size_t ind) {return pts[ind].coordinates[coord];}  ));


//create the initial centers
//choose the starting centers randomly
//the use of n/k guarantees we won't pick the same point twice as a center    
template <typename T> sequence<center<T>> create_centers(const parlay::sequence<point<T>>& pts,size_t n, size_t k, size_t d) {
    parlay::random_generator gen;
    //std::uniform_int_distribution<> dis(0, n/k);
    // sequence<size_t> starting_coords = tabulate(k,[&] (size_t i) {
    //     size_t r = 1;
    //     return (n/k)*i+r;
    // });
    sequence<size_t> starting_coords = randseq(n,k);

    std::cout << "starting coords" << std::endl;
    for (int i = 0; i < k; i++) {
        std::cout << starting_coords[i] << std::endl;
    }
    std::cout << std::endl;

    // Initialize centers randomly
    sequence<center<T>> centers(k);
    for (int i = 0; i < k; i++) {
        centers[i].coordinates = parlay::sequence<T>(d, 0);
        for (size_t j = 0; j < d; j++) {
            centers[i].coordinates[j] = pts[starting_coords[i]].coordinates[j];
        }
        centers[i].id=i;

    }
    return centers;
}

// std::pair<parlay::sequence<center<T>>, double> kmeans(parlay::sequence<point<T>> &v, parlay::sequence<center<T>> &centers, size_t k, size_t max_iterations){


// }

size_t sum_of_dist( slice<it,s> S, size_t n){
    if (n == 1){
        return *S.begin();
    }
    else{ 
        size_t m = n/2;
        size_t l, r;
        parlay::par_do([&] { l = sum_of_dist(S.cut(0,m),m);},  [&] {r = sum_of_dist(S.cut(m,n),n-m);});
        return l + r;  
    }

}


//just get the time out of kmeans
template <typename T> double kmeans_justtime(parlay::sequence<point<T>>& pts, size_t k, size_t max_iterations){
    return naive_kmeans(pts,k,max_iterations).second;
}

template <typename T> double naive_kmeans_stats(parlay::sequence<point<T>>& pts, size_t k, size_t max_iterations){

    parlay::internal::timer timer = parlay::internal::timer();
    timer.start();
    std::cout << "running" << std::endl;

     if(pts.size() == 0){ //default case
        return 1.0;//std::make_pair(sequence<center<T>>(2),1.0);
    }

    // STATS
    stats curr_stat;

    double epsilon=0.01;
    size_t d = pts[0].coordinates.size();
    size_t n = pts.size();

    sequence<center<T>> centers = create_centers(pts,n,k,d);

  int iterations = 0;

  double total_diff = 0;
  
  while (iterations < max_iterations) {

   
    std::cout << "iter" << iterations << std::endl;
    iterations++;

  //    std::cout << "center printing" << std::endl;
  // for (int i = 0; i < k; i++) {
  //   print_center(centers[i]);
  // }
  // std::cout << std::endl << std::endl;

    // Assign each point to the closest center
    parlay::parallel_for(0, pts.size(), [&](size_t i) {
      pair<size_t,size_t> closest_pair = closest_point_stats(pts[i], centers);
      pts[i].best = closest_pair.first
      curr_stat.dist_calls[iterations] += closest_pair.second
    });

    std::cout << "past closest pt\n";

    // Compute new centers
    sequence<center<T>> new_centers = compute_centers(pts, centers, k, d, n);

    // Check convergence
    total_diff = 0.0;
    for (int i = 0; i < k; i++) {
      double diff = distanceA(centers[i].coordinates, new_centers[i].coordinates,0,d);
      total_diff += diff;
    }

    sequence<double> dists(n);
    dists = parlay::map(pts, [&](point p)){
        return distanceA(p.coordinates, center[p.best].coordinates, 0, d);
    }

    curr_stat.SSE[iterations] = sum_of_dist(make_slice(dists));

    centers = std::move(new_centers);

  //  std::cout << "difs " << total_diff << " " << epsilon << std::endl;

    if (total_diff <= epsilon) {
      break;
    }

    std::cout << "stats of iteration " << iteration << ": " << std::endl << "Distance calls: " << 
    curr_stat.dist_calls[iteration] << std::endl << "SSE: " << curr_stat.SSE[iteration] << std::endl;
      
  }

   std::cout << "center printing" << std::endl;
  // for (int i = 0; i < k; i++) {
  //   print_center(centers[i]);
  // }
  std::cout << "Error" << total_diff << std::endl;
  std::cout << std::endl << std::endl;
    

  return timer.total_time();

    //return std::make_pair(centers,timer.total_time());

}
