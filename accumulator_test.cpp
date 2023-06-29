#include "accumulator.h"
#include "parlay/parallel.h"
#include "parlay/sequence.h"
#include "parlay/primitives.h"
#include "parlay/internal/get_time.h"

#include <iostream>
#include <atomic>

const size_t n = 10e8;

// main function for testing
int main() {
    size_t sequential_sum = 0;
    std::atomic<size_t> atomic_sum = 0;
    accumulator<size_t> acc;
    
    auto timer = parlay::internal::timer();

    // sequential sum
    timer.start();
    for (size_t i = 0; i < n; i++) {
        sequential_sum++;
    }
    std::cout << "Sequential sum: " << timer.next_time() << " (" << sequential_sum << ")" << std::endl;

    // atomic sum
    parlay::parallel_for(0, n, [&](size_t i) {
        atomic_sum++;
    });
    std::cout << "Atomic sum: " << timer.next_time() << " (" << atomic_sum << ")" << std::endl;

    // accumulator sum
    parlay::parallel_for(0, n, [&](size_t i) {
        acc.increment();
    });
    // acc.total();
    std::cout << "Accumulator sum: " << timer.next_time() << " (" << acc.total() << ")" << std::endl;
}