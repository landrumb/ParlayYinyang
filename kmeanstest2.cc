//testing file for naive kmeans method (in impshared_3.h)
//g++ kmeanstest2.cc -o ktest.out -std=c++17

#include "impshared_4.h"


void test_closest_point();
void test_anti_overflow_avg();
void test_compute_centers();
void test_repeat_point();
void test_repeat_point2();
void test_repeat_point3();
void test_create_centers();
void test_kmeans();

void point_test() {
    point<double> myP; //empty constructor call done automatically
    myP.coordinates=parlay::make_slice(sequence<double>(5,3));

    print_point(myP);
}

void center_test() {
    center<double> myCenter(1,5,sequence<double>(5,2));
    myCenter.add_point(1032);

    print_center(myCenter);

}

int main() {
    std::cout << "starting tests kmeans2" << std::endl;


    //point_test();
    //center_test();
    //test_closest_point();
    //test_anti_overflow_avg();
    //test_compute_centers();
    //test_repeat_point();
    //test_repeat_point2();
    //test_repeat_point3();
    //test_create_centers();
    test_kmeans();

}


//actually testing by functions this time! (incremental testing will actually save time here)
void test_closest_point() {

  //because centers don't move or copy must be declared like this
  sequence<center<double>> centers(3);
  centers[0].id=0;
  centers[0].dim=5;
  centers[0].coordinates=sequence<double>(5,2);

   centers[1].id=1;
  centers[1].dim=5;
  centers[1].coordinates=sequence<double>(5,10);

   centers[2].id=2;
  centers[2].dim=5;
  centers[2].coordinates=sequence<double>(5,50);


  point<double> pTest;
  pTest.coordinates=parlay::make_slice(sequence<double>(5,21));

  std::cout << "Distance: " << distanceA(pTest.coordinates,make_slice(centers[0].coordinates),0,centers[0].dim) << std::endl;
  std::cout << "Closest point is: " << closest_point(pTest,centers) << std::endl;

}

void test_anti_overflow_avg() {
  sequence<double> seq(1,3); // lol empty sequence bad
  seq[0]=2;
  std::cout << "averaging" << std::endl;
  std::cout << anti_overflow_avg(seq) << std::endl;
}

void test_compute_centers() {
  std::cout << "starting compute centers test" << std::endl;

  sequence<center<double>> centers(3);
  centers[0].id=0;
  centers[0].dim=5;
  centers[0].coordinates=sequence<double>(5,2);

   centers[1].id=1;
  centers[1].dim=5;
  centers[1].coordinates=sequence<double>(5,10);

   centers[2].id=2;
  centers[2].dim=5;
  centers[2].coordinates=sequence<double>(5,50);

    //create the points
    sequence<point<double>> pts(10);
  for (int i = 0; i < 10; i++) {
    
    pts[i].id=i;
    pts[i].best=i%2;
    sequence<double>* nsq = new sequence<double>(5,3*i);
    pts[i].coordinates=make_slice(*nsq);
    
  }


  std::cout << "pts: " << std::endl;
  for (int i = 0; i < 10; i++) {
    print_point(pts[i]);
  }

  sequence<center<double>> new_centers = compute_centers(pts,centers,3,5,10);
  std::cout << "printing centers " << std::endl;

  for (int i = 0; i < 3; i++) {
    print_center(new_centers[i]);
  }

}

//why does one work but not the other?? (why from_function)
void test_repeat_point() {
    std::cout << "testing the creation of a point array" << std::endl;
    sequence<point<double>> pts(10);
//   for (int i = 0; i < 10; i++) {
    
 
//     pts[i].best=i%3;
//     pts[i].coordinates=make_slice(sequence<double>(5,3*i));
//     //print_point(pts[0]);
//   }
  pts = sequence<point<double>>::from_function(10, [&] (size_t i) {point<double> p; p.coordinates=make_slice(sequence<double>(5,3*i+1.01));p.id =i; p.best=i%3; return p;});

  std::cout << "pts: " << std::endl;
  for (int i = 0; i < 10; i++) {
    print_point(pts[i]);
  }

}

//why does one work but not the other?? (why from_function)
void test_repeat_point2() {
    std::cout << "testing the creation of a point array 2" << std::endl;
    sequence<point<double>> pts(10);
  for (int i = 0; i < 10; i++) {
    point<double>* temp = new point<double>();
    temp[i].id=i;
    temp[i].best=i%3;
    sequence<double>* nsq = new sequence<double>(5,i*4+1);
    temp[i].coordinates=make_slice(*nsq);
    pts[i]=*temp;
    
  }

  std::cout << "pts: " << std::endl;
  for (int i = 0; i < 10; i++) {
    print_point(pts[i]);
  }

}


//why does one work but not the other?? (why from_function)
void test_repeat_point3() {
    std::cout << "testing the creation of a point array 3" << std::endl;
    sequence<point<double>> pts(10);
  for (int i = 0; i < 10; i++) {
    
    pts[i].id=i;
    pts[i].best=i%3;
    sequence<double>* nsq = new sequence<double>(5,i*4+1);
    pts[i].coordinates=make_slice(*nsq);
    
  }

  std::cout << "pts: " << std::endl;
  for (int i = 0; i < 10; i++) {
    print_point(pts[i]);
  }

}

void test_create_centers() {
    std::cout << "testing the creation of the centers" << std::endl;

     int n = 12;
    int d = 5;
    int k = 4;
    sequence<point<double>> pts(n);
   
    for (int i = 0; i < n; i++) {
    
        pts[i].id=i;
        pts[i].best=i%3;
        sequence<double>* nsq = new sequence<double>(d,i*4+1);
        pts[i].coordinates=make_slice(*nsq);
    
    }

    sequence<center<double>> centers = create_centers(pts,n,k,d);
    for (int i = 0; i < centers.size(); i++) {
        print_center(centers[i]);
    }

}

void test_kmeans() {

     int n = 12;
    int d = 5;
    int k = 4;
    int max_iter = 20;
    sequence<point<double>> pts(n);
   
    for (int i = 0; i < n; i++) {
    
        pts[i].id=i;
        pts[i].best=i%3;
        sequence<double>* nsq = new sequence<double>(d,i*i+1);
        pts[i].coordinates=make_slice(*nsq);
    
    }

    std::cout << "pts: " << std::endl;
  for (int i = 0; i < n; i++) {
    print_point(pts[i]);
  }

  

    auto mypair = naive_kmeans(pts,k,max_iter);

    std::cout << mypair << std::endl;


    
   
    // std::cout << "Time: " << timetaken << std::endl;
    // for (int i = 0; i < centers.size(); i++) {
    //     print_center(centers[i]);
    // }

}