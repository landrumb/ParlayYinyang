#include "accumulator.h"
#include "parlay/parallel.h"
#include "parlay/sequence.h"
#include "parlay/primitives.h"
#include "parlay/internal/get_time.h"

#include <iostream>
#include <atomic>
#include <random>

const size_t n = 10e9;

auto rng = std::mt19937(std::random_device{}());

int foo(int i) __attribute__((noinline)); 
int bar() __attribute__((noinline));

int foo(int i) {
    return i;
}

// this is slow because the rng is thread safe and has to be locked
int bar() {
    return rng() % 100;
}


// main function for testing
int main() {
    size_t sequential_sum = 0;
    std::atomic<size_t> atomic_sum = 0;
    accumulator<size_t> acc;
    
    auto timer = parlay::internal::timer();

    // sequential sum
    timer.start();
    for (size_t i = 1; i <= n; i++) {
        sequential_sum += foo(i);
    }
    std::cout << "Sequential sum: " << timer.next_time() << " (" << sequential_sum << ")" << std::endl;

    // atomic sum
    // parlay::parallel_for(1, n+1, [&](size_t i) {
    //     atomic_sum += foo(i);
    // });
    // std::cout << "Atomic sum: " << timer.next_time() << " (" << atomic_sum << ")" << std::endl;

    // accumulator sum
    parlay::parallel_for(1, n+1, [&](size_t i) {
        acc.add(foo(i));
    });
    // acc.total();
    std::cout << "Accumulator sum: " << timer.next_time() << " (" << acc.total() << ")" << std::endl;
}