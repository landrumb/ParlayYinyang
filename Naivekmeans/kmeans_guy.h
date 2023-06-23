//Adapting Guy's k means to our format, fixing bug(s)


// the 100 in the following two is for granularity control
// i.e. if the number of dimensions is less than 100, run sequentially
// template <typename T> point<T> operator/(const point& a, const double b) {
//     point c;
//     c.best = a.best;
//     c.coordinates = parlay::map(a.coordinates, [=] (T v) {return v/b;}, 100);
//     return c;
// }

// template <typename T> point<T> operator+(const point& a, const point& b) {
//   if (a.coordinates.size() == 0) return b; //not sure why this check is here
//   if (b.coordinates.size() == 0) return a; //I added this check just to be symmetrical
//   point c;
//   c.best = -1; //we shouldn't store the best in an add
//   c.coordinates = parlay::tabulate(a.coordinates.size(), [&] (long i) {return a[i] + b[i];}, 100);}
//   return c;

#include "types.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <set>
#include <atomic>
#include <mutex>

using namespace parlay;


template <typename T> double euclidean_squared(const sequence<T>& a, const sequence<T>& b) {
  return parlay::reduce(parlay::delayed::tabulate(a.size(), [&] (long i) {
    auto d = a[i]-b[i];
    return d*d;}));
}

template <typename T> double euclidean_squared(const slice<T*,T*>& a, const sequence<T>& b) {
  return parlay::reduce(parlay::delayed::tabulate(a.size(), [&] (long i) {
    auto d = a[i]-b[i];
    return d*d;}));
}


// the 100 in the following two is for granularity control
// i.e. if the number of dimensions is less than 100, run sequentially
template <typename T> sequence<T> operator/(const sequence<T>& a, const double b) {
  return parlay::map(a, [=] (T v) {return (T) (v/b);}, 100);}

template <typename T> sequence<T> operator/(const point<T>&a, const double b) {
    return parlay::map(a.coordinates, [=] (T v) {return (T) (v/b);}, 100);}

template <typename T> sequence<T> operator/(const slice<T*,T*>& a, const double b) {
    return parlay::map(a, [=] (T v) {return (T) (v/b);}, 100);}

template <typename T> sequence<T> operator+(const sequence<T>& a, const sequence<T>& b) {
  if (a.size() == 0) return b;
  return parlay::tabulate(a.size(), [&] (size_t i) {return (T) (a[i] + b[i]);}, 100);}

template <typename T> sequence<T> operator+(const sequence<T>& a, const slice<T*,T*>& b) {
  return parlay::tabulate(a.size(), [&] (size_t i) {return (T) (a[i] + b[i]);}, 100);}

template <typename T> sequence<T> operator+(const slice<T*,T*>& a, const slice<T*,T*>& b) {
  return parlay::tabulate(a.size(), [&] (size_t i) {return (T) (a[i] + b[i]);}, 100);}

template <typename T> sequence<T> operator+(const slice<T*,T*>& a, const sequence<T>& b) {
  return parlay::tabulate(a.size(), [&] (size_t i) {return (T) (a[i] + b[i]);}, 100);}



template <typename T> size_t guy_closest_point(const point<T>& p, const sequence<center<T>>& kpts) {
  auto a = parlay::delayed::map(kpts, [&] (const center<T> &q) {
    return euclidean_squared(p.coordinates, q.coordinates);});
  return min_element(a) - a.begin();
}

template <typename T> auto addpair = [] (const std::pair<sequence<T>,long> &a,
                   const std::pair<sequence<T>,long> &b) {
  return std::pair(a.first + b.first, a.second + b.second);};

template <typename T> auto addme = parlay::binary_op(addpair<T>, std::pair(sequence<T>(), 0l));


// template <typename T> sequence<std::pair<T,size_t>> my_seq_reduce_by_index(sequence<std::pair<point<T>,size_t>> closest, size_t k) {
//     sequence<std::pair<sequence<T>,long>> result(k);
//     for (size_t j = 0; j < closest.size(); j++) {
//         size_t key = closest[j].first.best;
// >>>>>>> 791a831 (forgot to commit, note Guy's code not ready yet)

template <typename T> sequence<std::pair<sequence<T>,size_t>> my_seq_reduce_by_index(sequence<std::pair<point<T>,size_t>> closest, size_t k, size_t d) {
    sequence<std::pair<sequence<T>,size_t>> result(k,std::make_pair(sequence<T>(d,0),1));
   
    for (size_t j = 0; j < closest.size(); j++) {
        size_t key = closest[j].second;
        result[key].second+=1;
        result[key].first = result[key].first + closest[j].first.coordinates;
    }
    return result;

}

// **************************************************************
// This is the main algorithm
// It uses k random points from the input as the starting centers
// **************************************************************

template <typename T> std::pair<sequence<center<T>>,double> guy_kmeans(sequence<point<T>>& pts, sequence<center<T>>& centers, size_t k, size_t max_iterations, double epsilon) {

    auto timer = parlay::internal::timer();
    timer.start();
  long n = pts.size();
  assert(n > 0); // need nonempty point size 
  size_t d = pts[0].coordinates.size();


  long round = 0;
  while (round < max_iterations) {
    // for each point find closest among the k centers (kpts)

    sequence<std::pair<point<T>,size_t>> closest = parlay::map(pts, [&] (const point<T>& p) {
      return std::pair{p,guy_closest_point(p, centers)};}); //p note p instead of a (p,1) pair
// =======
//     sequence<std::pair<size_t, point<T>>> closest = parlay::map(pts, [&] (const point<T>& p) {
//       return std::pair{guy_closest_point(p, centers),p};}); //p note p instead of a (p,1) pair
// >>>>>>> 791a831 (forgot to commit, note Guy's code not ready yet)
// =======
//     sequence<std::pair<point<T>,size_t>> closest = parlay::map(pts, [&] (const point<T>& p) {
//       return std::pair{p,guy_closest_point(p, centers)};}); //p note p instead of a (p,1) pair
// >>>>>>> f203cdd (trying to push)

    // Sum the points that map to each of the k centers
    //using a homemade reduce_by_index because
    //reduce_by_index(closest,k,addme<T>) 
    //gives a host of errors because the Monoid is templatted
    sequence<std::pair<sequence<T>,size_t>> mid_and_counts = my_seq_reduce_by_index(closest, k,d); 

    // Calculate new centers (average of the points)
    auto new_kpts = parlay::map(mid_and_counts, [&] (std::pair<sequence<T>,size_t> mcnt) {

      return (mcnt.first / (double) mcnt.second);});

    // compare to previous and quit if close enough
    auto ds = parlay::tabulate(k, [&] (long i) {
      return euclidean_squared(centers[i].coordinates, new_kpts[i]);});

    //copy over the new centers
    parallel_for(0,k,
        [&] (size_t i) {
            centers[i].coordinates=new_kpts[i];
        }
    );


    if (sqrt(parlay::reduce(ds)) < epsilon) break;

    
    round++;
  }
  return std::make_pair(centers,timer.total_time());
}
