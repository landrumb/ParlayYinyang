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

//g++ rand_init.cpp -o rand_init.out -std=c++17 -pthread

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


int main(){
    size_t k = 10;
    size_t n = 100;
    std::cout << "Array length: " << k << std::endl;
    sequence<size_t> K_centers(k);

    K_centers = randseq(n,k);

    for (int i = 0; i < k; i++){
        std::cout << K_centers[i] << std::endl;
    }
    
    
    return 0;

}