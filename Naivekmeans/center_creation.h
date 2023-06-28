//put the create centers code here so we don't redefine this function everywhere

#ifndef CENTER_CREATE
#define CENTER_CREATE

#include "types.h"

//create the initial centers
//choose the starting centers randomly
//the use of n/k guarantees we won't pick the same point twice as a center    
template <typename T> parlay::sequence<center<T>> create_centers(const parlay::sequence<point<T>>& pts,size_t n, size_t k, size_t d) {

    bool DEBUG_CENTER_CREATE = false;

    parlay::sequence<size_t> starting_coords = randseq(n,k);
    if (DEBUG_CENTER_CREATE) {
        std::cout << "starting coords" << std::endl;
        for (int i = 0; i < k; i++) {
            std::cout << starting_coords[i] << std::endl;
        }
        std::cout << std::endl;

    }
    

    // Initialize centers randomly
    parlay::sequence<center<T>> centers(k);
    for (int i = 0; i < k; i++) {
        centers[i].coordinates = parlay::sequence<T>(d);
        for (size_t j = 0; j < d; j++) {
            centers[i].coordinates[j] = pts[starting_coords[i]].coordinates[j];
        }
        centers[i].id=i;

    }
    return centers;
}


#endif //CENTER_CREATE