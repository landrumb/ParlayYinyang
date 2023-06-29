#include "types.h"


#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <set>
#include <atomic>
#include <mutex>

using namespace std;

template<typename T>
void to_csv(parlay::sequence<point<T>>& pts, parlay::sequence<center<T>>& centers, size_t n, size_t k, size_t d){

    cout << " WE REACHED HERE";

        // Open the file for writing
    ofstream file("data.csv");

    file << n << ", " << k << ", " << d << "\n";
 
    for(int i = 0; i < n; i++){
        for(int j = 0; j < d + 1; j++){
            if (j == d){file << pts[i].best << ", ";}
        
            else {file << pts[i].coordinates[j] << ", ";  }
        }
        file << '\n';
    }
    
    for(int i = 0; i < k; i++){
        for(int j = 0; j < d; j++){
           file << centers[i].coordinates[j] << ", ";
        }
        file << '\n';
    }

    
    
    // Close the file
    file.close();

    cout << "CSV file created successfully.\n";

}

// NOTE STORE THE N,K,D AT THE BEGINNING OF THE CSV