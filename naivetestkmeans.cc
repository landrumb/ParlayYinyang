//testing framework for the implementation in implementations_shared
#include "types.h"
#include "implementations_shared.h"
//#include "impshared_2.h"


// void test_closest_point();
// void test_anti_overflow_avg();
// void test_compute_centers();
// void test_repeat_point();
// //g++ naivetestkmeans.cc -o naivetestkmeans.out -std=c++17

// template <typename T> void print_point(const point<T>& p) {
//   std::cout << "Po: " << p.best << std::endl;
//   for (T* i = p.coordinates.begin(); i != p.coordinates.end(); i = std::next(i) ) {
//     std::cout << *i << " ";
//   }
//   std::cout << std::endl;
// }

// template <typename T> void print_center(const center<T>& c) {
//   std::cout << "ID" << c.id << std::endl;
//   std::cout <<"COORDS ";
//   for (int i = 0; i < c.dim; i++) {
//     std::cout << c.coordinates[i] << " ";
//   }
//   std::cout<<"Members: ";
//   std::cout << std::endl; //lol lifehack change type based on error msg
//   for (std::set<size_t>::const_iterator i = c.points.begin(); i != c.points.end(); i=std::next(i)) {
//     std::cout << *i << " ";


//   }
//   std::cout << std::endl;
  
// }
//std::set<size_t>::const_iterator // lol

int main() {
  std::cout << "hi" << std::endl;

  size_t n=50; //number of points
  size_t d=1; //number of dimensions
  size_t k=3; //number of clusters we want
  size_t max_iterations = 100;
  double eps = 0.01;


  std::random_device randomize;
  std::mt19937 generate(randomize());
  std::uniform_real_distribution<> dis(0.0, 100.0);

  // point<double> myP; //empty constructor call done automatically
  // myP.coordinates=parlay::make_slice(sequence<double>(5,3));

  // print_point(myP);

  // center<double> myCenter(1,5,sequence<double>(5,2));
  // myCenter.add_point(1032);

  // print_center(myCenter);

  // test_closest_point();
  // test_anti_overflow_avg();
  // test_compute_centers();
  // test_repeat_point();

  //Old formatting

    parlay::sequence<PPoint<double>> pts(n);  // Example with 10 points

  // Generate random points
  for (int i = 0; i < n; i++) {
    pts[i].coordinates = parlay::sequence<double>(d, 0.0);
    for (int j = 0; j < d; j++) {
      pts[i].coordinates[j] = dis(generate);
    }
  }


    auto ans = naive_kmeans(pts,  k, max_iterations, eps);
    std::cout << ans.second << std::endl;


}


// //actually testing by functions this time! (incremental testing will actually save time here)
// void test_closest_point() {

//   //because centers don't move or copy must be declared like this
//   sequence<center<double>> centers(3);
//   centers[0].id=0;
//   centers[0].dim=5;
//   centers[0].coordinates=sequence<double>(5,2);

//   //incorrect ways to create centers:
//   //centers.push_back(std::ref(center<double>(0,5,sequence<double>(5,2)))); //why doesn't this work? c++ syntax error
//   // centers.push_back(center<double>(1,5,sequence<double>(5,10))); //also doesn't work
//   //centers.push_back(*(new center<double>(0,5,sequence<double>(5,2)))); //nope
//   // centers.push_back(center<double>(2,5,sequence<double>(5,50)));

//    centers[1].id=1;
//   centers[1].dim=5;
//   centers[1].coordinates=sequence<double>(5,10);

//    centers[2].id=2;
//   centers[2].dim=5;
//   centers[2].coordinates=sequence<double>(5,50);


//   point<double> pTest;
//   pTest.coordinates=parlay::make_slice(sequence<double>(5,21));

//   std::cout << "Distance: " << distanceA(pTest.coordinates,make_slice(centers[0].coordinates),0,centers[0].dim) << std::endl;
//   std::cout << "Closest point is: " << closest_point(pTest,centers) << std::endl;

// }

// void test_anti_overflow_avg() {
//   sequence<double> seq(1,3); // lol empty sequence bad
//   seq[0]=2;
//   std::cout << "averaging" << std::endl;
//   std::cout << anti_overflow_avg(seq) << std::endl;
// }

// void test_compute_centers() {
//   std::cout << "starting compute centers test" << std::endl;

//   sequence<center<double>> centers(3);
//   centers[0].id=0;
//   centers[0].dim=5;
//   centers[0].coordinates=sequence<double>(5,2);

//   //incorrect ways to create centers:
//   //centers.push_back(std::ref(center<double>(0,5,sequence<double>(5,2)))); //why doesn't this work? c++ syntax error
//   // centers.push_back(center<double>(1,5,sequence<double>(5,10))); //also doesn't work
//   //centers.push_back(*(new center<double>(0,5,sequence<double>(5,2)))); //nope
//   // centers.push_back(center<double>(2,5,sequence<double>(5,50)));

//    centers[1].id=1;
//   centers[1].dim=5;
//   centers[1].coordinates=sequence<double>(5,10);

//    centers[2].id=2;
//   centers[2].dim=5;
//   centers[2].coordinates=sequence<double>(5,50);

//   sequence<point<double>> pts = sequence<point<double>>::from_function(10, [&] (size_t i) {point<double> p; p.coordinates=make_slice(sequence<double>(5,3*i)); p.best=i%3; return p;});

//   for (int i = 0; i < 10; i++) {
//     //sequence<double> temp(5,3*i);
//     pts[i].best=i%3;
//     pts[i].coordinates=make_slice(sequence<double>(5,3*i));
//   }

//   std::cout << "pts: " << std::endl;
//   for (int i = 0; i < 10; i++) {
//     print_point(pts[i]);
//   }

//   sequence<center<double>> new_centers = compute_centers(pts,3,5,10);
//   std::cout << "printing centers " << std::endl;

//   for (int i = 0; i < 3; i++) {
//     print_center(new_centers[i]);
//   }

// }

// //failing for some reason ... (all values copied into last one)
// // void test_repeat_point() {
// //     sequence<point<double>> pts(10,point<double>());
// //   for (int i = 0; i < 10; i++) {
// //     //sequence<double>* temp = new sequence<double>(5,3*i);
// //     //std::cout << temp[0] << std::endl;
// //     pts[i].best=i%3;
// //     pts[i].coordinates=make_slice(sequence<double>(5,3*i));
// //     print_point(pts[0]);
// //   }

// //   std::cout << "pts: " << std::endl;
// //   for (int i = 0; i < 10; i++) {
// //     print_point(pts[i]);
// //   }

// // }


// //why does one work but not the other?? (why from_function)
// void test_repeat_point() {
//     sequence<point<double>> pts(10);
//   // for (int i = 0; i < 10; i++) {
 
//   //   pts[i].best=i%3;
//   //   pts[i].coordinates=make_slice(sequence<double>(5,3*i));
//   //   //print_point(pts[0]);
//   // }
//   pts = sequence<point<double>>::from_function(10, [&] (size_t i) {point<double> p; p.coordinates=make_slice(sequence<double>(5,3*i)); p.best=i%3; return p;});

//   std::cout << "pts: " << std::endl;
//   for (int i = 0; i < 10; i++) {
//     print_point(pts[i]);
//   }

// }