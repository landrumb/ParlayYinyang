//testing framework for the implementation in implementations_shared

#include "implementations_shared.h"

//g++ naivetestkmeans.cc -o naivetestkmeans.out -std=c++17


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