#include "parlay/parallel.h"
#include "parlay/primitives.h"
#include "parlay/sequence.h"
#include "parlay/slice.h"
#include "parlay/random.h"
#include "parlay/internal/get_time.h"
//#include "NSGDist.h"
//#include "parse_files.h"
#include "types.h"

#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <set>
#include <atomic>


struct Distance {
    int a;

};

//std::pair<parlay::sequence<center<T>>,double>
template <typename T> auto naive_kmeans(parlay::sequence<point<T>>& v, size_t k, size_t max_iterations, Distance& D){
    auto timer = parlay::internal::timer();
    timer.start();

    if (v.size()==0) { //no pts given to sort
        return std::make_pair(parlay::sequence<center<T>>(),1.0);

    }

    return std::make_pair(parlay::sequence<center<T>>(),1.0);

}


// //parallel version
// //vectorization help plz
// //returns a sequence containing the final center choices
// //so the output should have size k*d
// //fixme: add functionality to have variable metrics, not just the dist function I've defined below (issues with including it)
// template <typename T> parlay::sequence<T> par_kmeansA(const parlay::sequence<T>& A, size_t n, size_t d, size_t k, double threshold, size_t numIters) {


//     //hold the centers
//     //indices i*d, ..., (i+1)*d-1 hold the d coords of the ith center
//     parlay::sequence<T> centers(k*d);

//     //hold the membership IDs for all the points
//     //clusterID[i] gives the id (index) of the center that is closest to point i
//     parlay::sequence<int> clusterID(n);
//     //holds the sums of all of the elements in the cluster (for finding the mean)
//     //indices i*d, ..., (i+1)*d-1 hold the running sum of the points that belong to cluster i
//     parlay::sequence<T> cluster_sums(k*d);

//     //counts the number of points in each cluster, for the mean calculation
//     //num_in_cluster[i] gives the number of points in cluster i
//     parlay::sequence<int> num_in_cluster(k);

//     double old_sse = 0;

   


//     //we make the first k points of A the initial values of the centers
//     //note that this is probably a bad choice of centers, this could be improved
//     parlay::parallel_for(0,k * d, [&] (size_t i) {
//         centers[i] = A[i];
//     });

//     //for each iteration
//     for (int iter = 0; iter < numIters; iter++) {

//         std::cout << "starting assignment\n";
//         sequence_print(centers);
//         sequence_print(clusterID);

//         //assignment
//         parlay::sequence<T> distCandidates(n*k);//not great for runtime ... (n*k too big spacewise for large use)

//         parlay::parallel_for(0,n, [&] (size_t pt) { //do all the points in parallel
//             parlay::sequence<T> distCandidates(k,-1);
//             parlay::parallel_for(0,k, [&] (size_t cl) { //in parallel, calculate the distance from a point to all centers
//                 distCandidates[pt*k+cl] = l2_fromarray_par(A,pt,centers,cl,d,0,d);
//             });
          
            
//             size_t best = get_min_index(distCandidates,pt*k,(pt+1)*k); //use the min distance you found to determine the point's new membership
            
//             clusterID[pt]=best%k;

//         });

//         //update
//         //reset the number in each cluster to 0
//         parlay::parallel_for(0,n,[&] (size_t i) {
//             num_in_cluster[i] = 0;
//         });
        
//         //reset cluster sums to 0
//         parlay::parallel_for(0,n*d,[&] (size_t i) {
//             cluster_sums[i] = 0;
//         });

//         //calculate the cluster sums
//         for (int pt = 0; pt < n; pt++) {
//             parlay::parallel_for(0,d,[&] (size_t coord) {
//              cluster_sums[clusterID[pt]*d+coord] += A[pt*d+coord];


//             });
//             num_in_cluster[clusterID[pt]] += 1;

//         }
     
//         // std::cout << "printing cluster sums:";
//         // sequence_print(cluster_sums); 
//         // sequence_print(num_in_cluster); std::cout << std::endl;


//         // //divide by the number of elements in the cluster (the division part of a mean)
//         parlay::parallel_for(0,k*d, [&] (size_t clcoord) {
//             int cl = clcoord/d;
//             if (num_in_cluster[cl]!=0) {
//                 cluster_sums[clcoord]=cluster_sums[clcoord]/num_in_cluster[cl];
//             }
//         }
        
//         );

//         //assign the centers to the new means

//         parlay::parallel_for(0,k*d,[&] (size_t clcoord) {
//             // int cl = clcoord / d;
//             // int coord = clcoord % d;
//             //invariant: clcoord = cl*d + coord
//             centers[clcoord] = cluster_sums[clcoord];
//         });




//         //check finishing condition
//         if (iter==0){ 
//             old_sse = sse_par(A,clusterID,centers,n,d,k,0,n) ;
//             continue;
//         }
//         else {
//             double new_sse = sse_par(A,clusterID,centers,n,d,k,0,n) ;

//             assert (old_sse - new_sse >= -1); //kmeans should improve every step
//             if (old_sse - new_sse <= .5) { //if we didn't improve by much
//                 std::cout << "done early\n";
//                 return centers;

//             }
//             old_sse = new_sse;
//         }
       
//         //std::cout << std::endl << std::endl << std::endl;

//     }

//     return centers;

    
// }

// //compute the sse (Sum of squared errors) of a certain cluster set
// template<typename T> double sse_par(const parlay::sequence<T>& A, const parlay::sequence<int>& clusterID, const parlay::sequence<T>& centers, size_t n, size_t d, size_t k,size_t nstart, size_t nend) {
    
//     if (nend-nstart==0) {
//         return 0;
//     }
//     else if (nend-nstart == 1) {
//         return l2_fromarray_par(A,nstart,centers,clusterID[nstart],d,0,d);
//     }
//     else if (nend-nstart==2) {
//         return l2_fromarray_par(A,nstart,centers,clusterID[nstart],d,0,d) +  l2_fromarray_par(A,nstart+1,centers,clusterID[nstart+1],d,0,d); 
//     }
//     else {
//         double left_sum = 0;
//         double right_sum = 0;
//         size_t mid = (nstart+nend)/2;
//         parlay::par_do(
//             [&]() {left_sum = sse_par(A,clusterID,centers,n,d,k,nstart,mid);},
//             [&]() {right_sum = sse_par(A,clusterID,centers,n,d,k,mid,nend);}
//         );
//         return left_sum+right_sum;
//     }

// }

// //take your sequences direct from the larger array (which is how we are storing them anyway)
// //this version works in parallel
// template <typename T> double l2_fromarray_par(const parlay::sequence<T>& A, int a, const parlay::sequence<T>& B, int b, int d, int dstart, int dend) {
    
//     if (dend-dstart==0){
//         return 0;
//     }
//     else if (dend-dstart==1) {
//         double err = (A[a*d+dstart]-B[b*d+dstart]); //avoid the double array access by storing err first then multiplying err*err
//         return err*err;
//     }
//     else if (dend-dstart==2) {
//         double err1 = (A[a*d+dstart]-B[b*d+dstart]); //avoid the double array access by storing err first then multiplying err*err
//         double err2 = (A[a*d+dstart+1]-B[b*d+dstart+1]);
//         return err1*err1+err2*err2;
//     }
//     else {
//         double left_sum=0;
//         double right_sum=0;
//         int mid = (dstart+dend)/2;

//         parlay::par_do (
//         [&]() {left_sum = l2_fromarray_par(A, a,B,b,d,dstart, mid);},
//         [&]() {right_sum = l2_fromarray_par(A, a,B,b,d,mid, dend);}
//         );

//         return left_sum+right_sum;
//     }
   
// }

// //Q: how to get in parlay?
// //homegrown
// //min function that runs parallel and returns an index
// template <typename T> size_t get_min_index(const parlay::sequence<T>& A, size_t start, size_t end) {
//     if (end-start==0){ 
//         return 0;
//     }
//     else if (end-start==1) {
//         return start;
//     }
//     else if (end-start == 2) {
//         if (A[start] < A[start+1]) return start;
//         return start+1;
//     }
//     size_t left_min_index = 0;
//     size_t right_min_index = 0;
//     size_t mid = (start+end)/2;
//     parlay::par_do (
//         [&]() {left_min_index = get_min_index(A, start, mid);},
//         [&]() {right_min_index = get_min_index(A,mid, end);}
//     );
    
//     if (A[left_min_index] < A[right_min_index]) {
//         return left_min_index;
//     }
//     else {
//         return right_min_index;
//     }
// }

// //parallel calculate the number of pts in all clusters
// //by splitting into adding from the two halves
// void cluster_nums_par(const parlay::sequence<int>& clusterID, parlay::sequence<int>& num_in_cluster, size_t start, size_t end) {
   

//     if (end-start==1) {
//         num_in_cluster[clusterID[start]] += 1;
//     }
//     else if (end-start > 1) {
//         int mid = (start+end)/2;
//         parlay::par_do( 

//             [&]() {cluster_nums_par(clusterID,num_in_cluster,start,mid);},
//             [&]() {cluster_nums_par(clusterID,num_in_cluster,mid,end);});

//     }
// }
