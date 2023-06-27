//Code to compare the speeds of our distance function, Guy's, and the vectorized
//distance from NSGdist.h

#include "NSGDist.h"
#include "types.h"
#include <stdlib.h>


parlay::sequence<float> randFloats(size_t n, size_t maxval) {
    parlay::sequence<float> result;
    for (int i = 0; i < n; i++) {
        result.push_back(std::rand() % maxval);
    }
    return result;
}



//hmm have to pass by value? why? (double vs const double error)
//version of distance that works with slices
template <typename T> double distanceA(parlay::slice<T*,T*> p1, parlay::slice<T*,T*> p2,size_t dstart, size_t dend) {

    //std::cout << "dist: " << dstart << " " << dend << std::endl;
    
    assert(dstart <= dend); //the end should never be less than the start

    if (dend-dstart==0){
        return 0;
    }
    else if (dend-dstart==1) {
        //std::cout << "first elt of the slices: " << p1[dstart] << " " << p2[dstart] << std::endl;
        double err = p1[dstart] - p2[dstart]; //avoid the double array access by storing err first then multiplying err*err
        //std::cout << "error is " << err << "squared: " << err * err << std::endl;
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

       // std::cout << "sums: " << left_sum << " " << right_sum << std::endl;


        return left_sum+right_sum;
    }
   
}


template <typename T>
double euclidean_squared(const parlay::slice<T*, T*>& a, const parlay::slice<T*,T*>& b) {
  return parlay::reduce(parlay::delayed::tabulate(a.size(), [&](long i) {
    auto d = a[i] - b[i];
    return d * d;
  }));
}



template<typename T>
double nsg_distance(const parlay::slice<T*,T*>& a, const parlay::slice<T*,T*> b) {
    Distance* D = new Euclidian_Distance();
    assert(a.size()==b.size()); //two slices should be of the same size
    std::cout << "Size of a: " << a.size() << std::endl;
    return static_cast<double>(D->distance(a.begin(), b.begin(), a.size()));
}



int main() {
    int n = 2;
    int maxval = 100;

    srand(time(0));

    parlay::sequence<float> mySeq = randFloats(n, maxval); 
    print_seq(mySeq);

    auto mySeq2 = randFloats(n, maxval); //numbers from 0 to 100, 3 random numbers
    print_seq(mySeq2);

    auto mySlice = make_slice(mySeq);
    auto mySlice2 = make_slice(mySeq2);


    std::cout << "euc dist" << euclidean_squared(mySlice,mySlice2) << std::endl;
    std::cout << "A dist " << distanceA(mySlice,mySlice2,0,mySeq.size()) << std::endl;

    std::cout << "nsg " << nsg_distance(mySlice,mySlice2) << std::endl;

    std::cout << "size: " << mySeq.size() << std::endl;



}
