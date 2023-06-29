//Code to compare the speeds of our distance function, Guy's, and the vectorized
//distance from NSGdist.h

#include "NSGDist.h"
#include "types.h"
#include <stdlib.h>

// parlay::sequence<double> randDoubles(size_t n, size_t maxval, parlay::random_generator gen) {
//     std::uniform_real_distribution<double> dis(0, maxval-1);
//     auto result = parlay::tabulate(n, [&](size_t i) {
//     auto r = gen[i];
//     return dis(r);
//     });
//     return result;
// }

// parlay::sequence<float> randFloats(size_t n, size_t maxval, parlay::random_generator gen) {
//     std::uniform_real_distribution<float> dis(0, maxval-1);
//     auto result = parlay::tabulate(n, [&](size_t i) {
//     auto r = gen[i];
//     return dis(r);
//     });
//     return result;
// }
// parlay::sequence<float> randFloats(size_t n, size_t maxval, parlay::random_generator gen) {
//     parlay::sequence<float> result;
//     std::uniform_real_distribution<float> dis(0, maxval-1);
//     for (int i = 0; i < n; i++) {
//         result.push_back(dis(gen));
//     }
//     return result;
// }

parlay::sequence<float> randFloats(size_t n, size_t maxval) {
    parlay::sequence<float> result;
    for (int i = 0; i < n; i++) {
        result.push_back(std::rand() % maxval);
    }
    return result;
}

//take your sequences direct from the larger array (which is how we are storing them anyway)
//this is the sequential version
template <typename T> double l2_fromarray(const parlay::sequence<T>& A, int a, parlay::sequence<T>& B, int b, int d) {
    double sum = 0;
    for (int i = 0; i < d; i++) {
        sum += (A[a*d+i]-B[b*d+i])*(A[a*d+i]-B[b*d+i]);
    }
    return sum;
}
//take your sequences direct from the larger array (which is how we are storing them anyway)
//this is the sequential version
template <typename T> double l2_fromarray(const parlay::slice<T*,T*>& A, parlay::slice<T*,T*>& B, int d) {
    double sum = 0;
    for (int i = 0; i < d; i++) {
        sum += (A[i]-B[i])*(A[i]-B[i]);
    }
    return sum;
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


//does adding base cases make the algorithm much faster on small cases and essentially the same on large cases?
template <typename T>
double es_base(const parlay::slice<T*, T*>& a, const parlay::slice<T*,T*>& b) {
    if (a.size() == 1) {
        auto d = a[0]-b[0];
        return d*d;
    }
    else if (a.size()==2) {
        auto d = a[0]-b[0];
        auto d2 = a[1]-b[1];
        return d*d+d2*d2;
    }
  return parlay::reduce(parlay::delayed::tabulate(a.size(), [&](long i) {
    auto d = a[i] - b[i];
    return d * d;
  }));
}

//es double index
template <typename T>
double es_doubleindex(const parlay::slice<T*, T*>& a, const parlay::slice<T*,T*>& b) {
  return parlay::reduce(parlay::delayed::tabulate(a.size(), [&](long i) {
    return (a[i] - b[i])*(a[i]-b[i]);//doesn't store anything but uses more index calls
  }));
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
    //std::cout << "Size of a: " << a.size() << std::endl;
    return static_cast<double>(D->distance(a.begin(), b.begin(), a.size()));
}


//n -- array size
//max_val -- maximum value an array elt can take
//iter -- number of iterations to do
double test_e2(int iter, double max_val, size_t n) {
    parlay::internal::timer t;
    for (int i = 0; i < iter; i++) {
        parlay::sequence<float> seq = randFloats(n,max_val);
        parlay::slice<float*,float*> sli = make_slice(seq);
        parlay::sequence<float> seq2 = randFloats(n,max_val);
        parlay::slice<float*,float*> sli2 = make_slice(seq);
        t.start();
        euclidean_squared(sli,sli2);
        t.stop();
    }
    return t.total_time();
}

//n -- array size
//max_val -- maximum value an array elt can take
//iter -- number of iterations to do
double test_dA(int iter, double max_val, size_t n) {
    parlay::internal::timer t;
    for (int i = 0; i < iter; i++) {
        parlay::sequence<float> seq = randFloats(n,max_val);
        parlay::slice<float*,float*> sli = make_slice(seq);
        parlay::sequence<float> seq2 = randFloats(n,max_val);
        parlay::slice<float*,float*> sli2 = make_slice(seq);
        t.start();
        distanceA(sli,sli2,0,n);
        t.stop();
    }
    return t.total_time();
}


//test Euclidian squared and distanceA using the same random values, for more precise results 
//note we aren't counting the time it takes to create the slices, just the calls
std::pair<double, double> test_together(int iter, double max_val, size_t n) {
    parlay::internal::timer tE;
    parlay::internal::timer tA;
    for (int i = 0; i < iter; i++) {
        parlay::sequence<float> seq = randFloats(n,max_val);
        parlay::slice<float*,float*> sli = make_slice(seq);
        parlay::sequence<float> seq2 = randFloats(n,max_val);
        parlay::slice<float*,float*> sli2 = make_slice(seq);
        tA.start();
        double ans1 = distanceA(sli,sli2,0,n);
        tA.stop();
        tE.start();
        double ans2 = euclidean_squared(sli,sli2);
        tE.stop();
        assert(ans1==ans2); //answers should be equal
    }

    return std::make_pair(tE.total_time(),tA.total_time());

}

//test Euclidian squared and distanceA using the same random values, for more precise results 
//note we aren't counting the time it takes to create the slices, just the calls
std::pair<double, double> test_together_euc_vec(int iter, double max_val, size_t n) {
    parlay::internal::timer tE;
    parlay::internal::timer tA;
    for (int i = 0; i < iter; i++) {
        parlay::sequence<float> seq = randFloats(n,max_val);
        parlay::slice<float*,float*> sli = make_slice(seq);
        parlay::sequence<float> seq2 = randFloats(n,max_val);
        parlay::slice<float*,float*> sli2 = make_slice(seq);
        tA.start();
        double ans1 = nsg_distance(sli,sli2);
        tA.stop();
        tE.start();
        double ans2 = euclidean_squared(sli,sli2);
        tE.stop();
        assert(ans1==ans2); //answers should be equal
    }

    return std::make_pair(tE.total_time(),tA.total_time());

}

std::pair<double, double> test_together_otherorder(int iter, double max_val, size_t n) {
    parlay::internal::timer tE;
    parlay::internal::timer tA;
    for (int i = 0; i < iter; i++) {
        parlay::sequence<float> seq = randFloats(n,max_val);
        parlay::slice<float*,float*> sli = make_slice(seq);
        parlay::sequence<float> seq2 = randFloats(n,max_val);
        parlay::slice<float*,float*> sli2 = make_slice(seq);
        
        tE.start();
        double ans2 = euclidean_squared(sli,sli2);
        tE.stop();

        tA.start();
        double ans1 = distanceA(sli,sli2,0,n);
        tA.stop();

        assert(ans1==ans2); //answers should be equal
    }

    return std::make_pair(tE.total_time(),tA.total_time());

}

std::pair<double, double> test_together_samefunction(int iter, double max_val, size_t n) {
    parlay::internal::timer tE;
    parlay::internal::timer tA;
    for (int i = 0; i < iter; i++) {
        parlay::sequence<float> seq = randFloats(n,max_val);
        parlay::slice<float*,float*> sli = make_slice(seq);
        parlay::sequence<float> seq2 = randFloats(n,max_val);
        parlay::slice<float*,float*> sli2 = make_slice(seq);
        
        tE.start();
        double ans2 = euclidean_squared(sli,sli2);
        tE.stop();

        tA.start();
        double ans1 = euclidean_squared(sli,sli2);
        tA.stop();

        assert(ans1==ans2); //answers should be equal
    }

    return std::make_pair(tE.total_time(),tA.total_time());

}

std::pair<double, double> test_together_diffseq(int iter, double max_val, size_t n) {
    parlay::internal::timer tE;
    parlay::internal::timer tA;
    for (int i = 0; i < iter; i++) {
        parlay::sequence<float> seq = randFloats(n,max_val);
        parlay::slice<float*,float*> sli = make_slice(seq);
        parlay::sequence<float> seq2 = randFloats(n,max_val);
        parlay::slice<float*,float*> sli2 = make_slice(seq);
        
        tE.start();
        double ans2 = euclidean_squared(sli,sli2);
        tE.stop();

        parlay::sequence<float> seq3 = randFloats(n,max_val);
        parlay::slice<float*,float*> sli3 = make_slice(seq);
        parlay::sequence<float> seq4 = randFloats(n,max_val);
        parlay::slice<float*,float*> sli4 = make_slice(seq);

        tA.start();
        double ans1 = distanceA(sli3,sli4,0,n);
        tA.stop();

        assert(ans1==ans2); //answers should be equal
    }

    return std::make_pair(tE.total_time(),tA.total_time());

}

//Question: does the same sequence take differing run times? 

void sameSeqdiffTimes() {
    int iter = 100;
    int n = 100000000;
    parlay::internal::timer tE;
    int max_val = 1'000'000'000;

    parlay::sequence<float> seq = randFloats(n,max_val);
    parlay::slice<float*,float*> sli = make_slice(seq);
    parlay::sequence<float> seq2 = randFloats(n,max_val);
    parlay::slice<float*,float*> sli2 = make_slice(seq);

    tE.start();

    for (int i = 0; i < iter; i++) {
        euclidean_squared(sli,sli2);

        

        tE.next("run: ");


    }
}


//test Euclidian squared and distanceA using the same random values, for more precise results 
//note we aren't counting the time it takes to create the slices, just the calls
std::tuple<double, double,double> test_3(int iter, double max_val, size_t n) {
    parlay::internal::timer tE;
    parlay::internal::timer tA;
    parlay::internal::timer tC;

    for (int i = 0; i < iter; i++) {
        parlay::sequence<float> seq = randFloats(n,max_val);
        parlay::slice<float*,float*> sli = make_slice(seq);
        parlay::sequence<float> seq2 = randFloats(n,max_val);
        parlay::slice<float*,float*> sli2 = make_slice(seq);
     
        tC.start();
        double ans2 = es_base(sli,sli2);
        tC.stop();

        tE.start();
        double ans3  = es_doubleindex(sli,sli2);
        tE.stop();


        tA.start();
        double ans1 = euclidean_squared(sli,sli2);
        tA.stop();
            

       
        assert(ans1==ans2); //answers should be equal
        assert(ans2==ans3);
    }

    return std::make_tuple(tA.total_time(),tC.total_time(),tE.total_time());

}

std::pair<double,double> seq_vs_par(int iter, double max_val, size_t n) {

     parlay::internal::timer tE;
    parlay::internal::timer tA;
    for (int i = 0; i < iter; i++) {
        parlay::sequence<float> seq = randFloats(n,max_val);
        parlay::slice<float*,float*> sli = make_slice(seq);
        parlay::sequence<float> seq2 = randFloats(n,max_val);
        parlay::slice<float*,float*> sli2 = make_slice(seq);
        tA.start();
        double ans1 = l2_fromarray(sli,sli2,n);
        tA.stop();
        tE.start();
        double ans2 = euclidean_squared(sli,sli2);
        tE.stop();
        assert(ans1==ans2); //answers should be equal
    }

    return std::make_pair(tE.total_time(),tA.total_time());

}







int main() {
    // int n = 2;
    // int maxval = 100;

    // srand(time(0));

    // parlay::sequence<float> mySeq = randFloats(n, maxval); 
    // print_seq(mySeq);

    // auto mySeq2 = randFloats(n, maxval); //numbers from 0 to 100, 3 random numbers
    // print_seq(mySeq2);

    // auto mySlice = make_slice(mySeq);
    // auto mySlice2 = make_slice(mySeq2);



    // std::cout << "euc dist" << euclidean_squared(mySlice,mySlice2) << std::endl;
    // std::cout << "A dist " << distanceA(mySlice,mySlice2,0,mySeq.size()) << std::endl;
    // std::cout << "size: " << mySeq.size() << std::endl;

    // std::cout << "nsg " << nsg_distance(mySlice,mySlice2) << std::endl;

    // std::cout << "Test e2: n=1000: " << test_e2(1000,1000000000,1000) << std::endl;
    // std::cout << "Test dA: n=1000: " << test_dA(1000,1000000000,1000) << std::endl;

    // std::cout << "Test e2: n=100: " << test_e2(10000,1000000000,100) << std::endl;
    // std::cout << "Test dA: n=100: " << test_dA(10000,1000000000,100) << std::endl;

    // auto pa = test_together(1000,1000000000,100);
    // std::cout << "results together, 1000-100 E: " << pa.first << " " << "A: " << pa.second << std::endl;


    // auto pa1 = test_together(1000,1000000000,1000);
    // std::cout << "results together, 1000-100 E: " << pa1.first << " " << "A: " << pa1.second << std::endl;


    // auto pa2 = test_together(1000,1000000000,10000);

    // std::cout << "results together, 1000-10000 E: " << pa2.first << " " << "A: " << pa2.second << std::endl;


    // auto pa3 = test_together(1000,1000000000,100000);

    // std::cout << "results together, 1000-100000 E: " << pa3.first << " " << "A: " << pa3.second << std::endl;

    int iters[3] = {1000,10000,100'000};//,100000};//100'000,1'000'000};
    int lens[3] = {100,1000,10'000};//,10000,1'000'000,1'000'000'000};
    int maxval = 1'000'000'000; // single quote ' is the separator for C++
    // std::cout << "test together \n";
    
    // for (int i : iters) {
    //     for (int len : lens) {
    //         auto pa = test_together(i,maxval,len);
    //         std::cout << "i: " << i << " len : " << len << " E: " << pa.first << " A: " << pa.second << " Ratio : " << pa.second/pa.first << std::endl;
    //     }
    // }

    // std::cout << "test together other order \n";

    // for (int i : iters) {
    //     for (int len : lens) {
    //         auto pa = test_together_otherorder(i,maxval,len);
    //         std::cout << "i: " << i << " len : " << len << " E: " << pa.first << " A: " << pa.second << " Ratio : " << pa.second/pa.first << std::endl;
    //     }
    // }

    // std::cout << "test together diff seq \n";

    // for (int i : iters) {
    //     for (int len : lens) {
    //         auto pa = test_together_diffseq(i,maxval,len);
    //         std::cout << "i: " << i << " len : " << len << " E: " << pa.first << " A: " << pa.second << " Ratio : " << pa.second/pa.first << std::endl;
    //     }
    // }
    //sameSeqdiffTimes();

    // for (int i : iters) {
    //     for (int len : lens) {
    //         auto pa = test_3(i,maxval,len);
    //         std::cout << "i: " << i << " len : " << len << " E: " << std::get<0>(pa) << " BA: " << std::get<1>(pa) << " DI: " << std::get<2>(pa) << std::endl;
    //     }
    // }

    //on 1 billion sized sequence, sequential is x1mil faster compared to Guy's implementation
    //seq vs par
    for (int i : iters) {
        for (int len : lens) {
            auto pa = seq_vs_par(i,maxval,len);
            std::cout << "i: " << i << " len : " << len << " Par: " << pa.first << " Seq: " << pa.second << std::endl;
        }
    }

    //test against itself
    //nvm


    //euc vs vec

    // int iters[2] = {100,10000};//,100000};//100'000,1'000'000};
    // int lens[5] = {100,1000,10000,1'000'000,1'000'000'000};//,10000,1'000'000,1'000'000'000};
    // int maxval = 1'000'000'000; // single quote ' is the separator for C++

    // for (int i : iters) {
    //     for (int len : lens) {
    //         auto pa = test_together_euc_vec(i,maxval,len);
    //         std::cout << "i: " << i << " len : " << len << " Euc: " << pa.first << " Vec: " << pa.second << std::endl;
    //     }
    // }

    //same function (euc)

    // int iters[5] = {1,10,100,1000,10000};//,100000};//100'000,1'000'000};
    // int lens[5] = {1,10,100,1000,10000};//,10000,1'000'000,1'000'000'000};
    // int maxval = 1'000'000'000; // single quote ' is the separator for C++

    // for (int i : iters) {
    //     for (int len : lens) {
    //         auto pa = test_together_samefunction(i,maxval,len);
    //         std::cout << "i: " << i << " len : " << len << " Euc: " << pa.first << " Vec: " << pa.second << std::endl;
    //     }
    // }
    



}
