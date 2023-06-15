#include <iostream>
#include <string>
#include <random>
#include <algorithm>
#include <utility>
#include <cmath>
#include "parlay/primitives.h"
#include "parlay/random.h"
#include "parlay/sequence.h"
#include "parlay/delayed.h"
#include "parlay/io.h"


using namespace parlay;

//g++ k_means[papa].cpp -o k_means[papa].out -std=c++17


// Define a struct to represent a point with coordinates
struct Point {
  parlay::sequence<double> coordinates;
};

// Define a type for a sequence of points
using Points = parlay::sequence<Point>;

//function for calculating distance
double distance(const Point& p1, const Point& p2) {
  double squared_distance = 0.0;
  parlay::parallel_for(0, p1.coordinates.size(), [&](std::size_t i) {
    double diff = p1.coordinates[i] - p2.coordinates[i];
    squared_distance += diff * diff;
  });
  return std::sqrt(squared_distance);
}

//finds the closest point utilizing the minimum distance
int closest_point(const Point& p, const Points& centers) {
  double min_distance = std::numeric_limits<double>::max();
  int closest_index = 0;

   parlay::parallel_for(0, centers.size(), [&](std::size_t i) {
    double d = distance(p, centers[i]);
    if (d < min_distance) {
      min_distance = d;
      closest_index = i;
    }
  });

  return closest_index;
}

// Compute new centers based on the points assigned to each cluster
Points compute_centers(const Points& pts, const parlay::sequence<int>& assignments, int k) {
  // Initialize counts for each cluster and new centers
  parlay::sequence<int> counts(k, 0);
  Points centers(k);
  
  // Initialize the coordinates of each new center
  for (int i = 0; i < k; i++) {
    centers[i].coordinates = parlay::sequence<double>(pts[0].coordinates.size(), 0.0);
  }

  // Calculate the sum of coordinates for each cluster
   parlay::parallel_for(0, pts.size(), [&](std::size_t i) {
    int cluster = assignments[i];
    counts[cluster]++;
   parlay::parallel_for(0, pts[i].coordinates.size(), [&](std::size_t j){
      centers[cluster].coordinates[j] += pts[i].coordinates[j];
    });
  });

  // Calculate the average coordinates for each cluster
  for (int i = 0; i < k; i++) {
    for (std::size_t j = 0; j < centers[i].coordinates.size(); j++) {
    if(counts[i] != 0){
      centers[i].coordinates[j] /= counts[i];
    }
    }
  }

  return centers;
}

std::pair<Points, int> k_means(Points pts, int k, double epsilon, int max_iterations) {
  std::random_device randomize;
  std::mt19937 generate(randomize());
  std::uniform_real_distribution<> dis(0.0, 1.0);

  // Initialize centers randomly
  Points centers(k);
  for (int i = 0; i < k; i++) {
    centers[i].coordinates = parlay::sequence<double>(pts[0].coordinates.size(), 0.0);
    for (std::size_t j = 0; j < centers[i].coordinates.size(); j++) {
      centers[i].coordinates[j] = dis(generate);
    }
  }

  parlay::sequence<int> assignments(pts.size(), -1);
  int iterations = 0;
  bool converged = false;

  while (!converged && iterations < max_iterations) {
    converged = true;
    iterations++;

    // Assign each point to the closest center
    parlay::parallel_for(0, pts.size(), [&](int i) {
      int closest = closest_point(pts[i], centers);
      if (assignments[i] != closest) {
        assignments[i] = closest;
        converged = false;
      }
    });

    // Compute new centers
    Points new_centers = compute_centers(pts, assignments, k);

    // Check convergence
    double max_diff = 0.0;
    for (int i = 0; i < k; i++) {
      double diff = distance(centers[i], new_centers[i]);
      max_diff = std::max(max_diff, diff);
    }

    if (max_diff <= epsilon) {
      converged = true;
    }

    centers = std::move(new_centers);
  }

  return {centers, iterations};
}

int main() {
  Points pts(10);  // Example with 10 points
  int dimensions = 2;  // Number of dimensions

 std::random_device randomize;
 std::mt19937 generate(randomize());
 std::uniform_real_distribution<> dis(0.0, 10.0);

  // Generate random points
  for (int i = 0; i < 10; i++) {
    pts[i].coordinates = parlay::sequence<double>(dimensions, 0.0);
    for (int j = 0; j < dimensions; j++) {
      pts[i].coordinates[j] = dis(generate);
    }
  }

  int k = 2;  // Number of clusters
  double epsilon = 0.001;  // Convergence threshold
  int max_iterations= 100;  // Maximum number of iterations

  std::pair<Points, int> result = k_means(pts, k, epsilon, max_iterations);

  // Print centers and number of iterations
  std::cout << "Centers:\n";
  for (const Point& center : result.first) {
    for (double coord : center.coordinates) {
      std::cout << coord << " ";
    }
    std::cout << "\n";
  }
  std::cout << "Iteratons: " << result.second << "\n";

  return 0;
}







