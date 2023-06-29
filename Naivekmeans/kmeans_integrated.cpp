//alternate file for running kmeans
//so the kmeans.cpp file can be used for running our original kmeans, impshared_4.h

#include "types.h"
#include "parse_files.h"
#include "parse_command_line.h"

#include "impshared_4.h"
#include "imp_with_euc_dist.h"
#include "imp_vectorized_distances.h"
#include "imp_doubled_centers.h"
#include "kmeans_guy.h"

#include "center_creation.h"

#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <set>
#include <atomic>
#include <utility>
#include <type_traits>





//this bench measures the amount of time naive kmeans, in impshared_4, takes
//the original implementation requires double points to work so we cast to doubles first
template <typename T>
void bench_original(parlay::sequence<point<T>> &v, size_t k, size_t m, double &runtime, bool DEBUG_main){
    assert(v.size() > 0); //want a nonempty point set
    size_t n = v.size();
    size_t d = v[0].coordinates.size();
    std::cout << "d: " << d << std::endl;
    std::cout << "Original: " << std::endl;

    //forcibly making some doubles here
    sequence<point<double>> v2(n);
    for (int i = 0; i < n; i++) {
        v2[i] = point<double>();
        v2[i].best = -1;
        
        sequence<double>* temp = new sequence<double>(d);
        for (int j = 0; j < d; j++) {
            (*temp)[j] = static_cast<double>(v[i].coordinates[j]);
        } 
        v2[i].coordinates = make_slice(*temp);
    }

    parlay::sequence<center<double>> centers = create_centers(v2,n,k,d);
    
    if (DEBUG_main) {
        print_point(v[0]);
        print_point(v[1]);

    }
    

    //need to properly copy the centers because we are passing by ref
    parlay::sequence<center<double>> centers2(k,center<double>()); 
    for (int i = 0; i < k; i++) {
        centers2[i].id = centers[i].id;
        centers2[i].coordinates=sequence<double>(d);
        for (int j = 0; j < d; j++) {
            centers2[i].coordinates[j] = centers[i].coordinates[j];
        }
    }


    if (DEBUG_main) {
         std::cout << "centers: " << std::endl;
    for (int i = 0; i < k; i++) {
       print_center(centers[i]);
       print_center(centers2[i]);
        
    }

    }
   
    
    double epsilon = 0.01;

    std::cout << "started the first kmeans\n";;

    //double runtime1 = 73.2417;
    double runtime1 = naive_kmeans<double>(v2, centers,k, m,epsilon).second;
    std::cout << "finished the first kmeans\n";
    double runtime2 = guy_kmeans<double>(v2,centers2,k,m,epsilon).second;
    std::cout << "guy: " << runtime2 << ", us: " << runtime1 << std::endl;

    if (DEBUG_main) {
        std::cout << "centers: " << std::endl;
    for (int i = 0; i < k; i++) {
        std::cout << "i:\n";
        print_center(centers[i]);
        print_center(centers2[i]);
    }

    }
    

   
    return;
}

//this version benchmarks our implementation, just swapping out our distanceA function for euclidean_distance
template<typename T>
void bench_euc_dist_version(parlay::sequence<point<T>> &v, size_t k, size_t m, double &runtime, bool DEBUG_main){
    assert(v.size() > 0); //want a nonempty point set
    size_t n = v.size();
    size_t d = v[0].coordinates.size();

    std::cout << "Euc dist version " << std::endl;
    std::cout << "d: " << d << std::endl;

    //forcibly making some doubles here
    sequence<point<double>> v2(n);
    for (int i = 0; i < n; i++) {
        v2[i] = point<double>();
        v2[i].best = -1;
        
        sequence<double>* temp = new sequence<double>(d);
        for (int j = 0; j < d; j++) {
            (*temp)[j] = static_cast<double>(v[i].coordinates[j]);
        } 
        v2[i].coordinates = make_slice(*temp);
    }

    parlay::sequence<center<double>> centers = create_centers(v2,n,k,d);
    
    if (DEBUG_main) {
        print_point(v[0]);
        print_point(v[1]);

    }
    

    //need to properly copy the centers because we are passing by ref
    parlay::sequence<center<double>> centers2(k,center<double>()); 
    for (int i = 0; i < k; i++) {
        centers2[i].id = centers[i].id;
        centers2[i].coordinates=sequence<double>(d);
        for (int j = 0; j < d; j++) {
            centers2[i].coordinates[j] = centers[i].coordinates[j];
        }
    }


    if (DEBUG_main) {
         std::cout << "centers: " << std::endl;
    for (int i = 0; i < k; i++) {
       print_center(centers[i]);
       print_center(centers2[i]);
        
    }

    }
   
    
    double epsilon = 0.01;

    std::cout << "started the first kmeans\n";;

    //double runtime1 = 73.2417;
    double runtime1 = kmeans_euc_dist<double>(v2, centers,k, m,epsilon);
    std::cout << "finished the first kmeans\n";
    double runtime2 = guy_kmeans<double>(v2,centers2,k,m,epsilon).second;
    std::cout << "guy: " << runtime2 << ", us: " << runtime1 << std::endl;

    if (DEBUG_main) {
        std::cout << "centers: " << std::endl;
    for (int i = 0; i < k; i++) {
        std::cout << "i:\n";
        print_center(centers[i]);
        print_center(centers2[i]);
    }

    }
    

   
    return;
}

//this version benchmarks our implementation where points are cast to doubles whenever they are needed, instead of copying the dataset at the beginning
template <typename T>
void bench_no_double_conversion(parlay::sequence<point<T>> &v, size_t k, size_t m, double &runtime, bool DEBUG_main){
    assert(v.size() > 0); //want a nonempty point set
    size_t n = v.size();
    size_t d = v[0].coordinates.size();

    std::cout << "Doubled centers\n";
    std::cout << "d: " << d << std::endl;

    parlay::sequence<center<T>> centers = create_centers(v,n,k,d);
    if (DEBUG_main) {
        print_point(v[0]);
        print_point(v[1]);

    }
    
    //need to properly copy the centers because we are passing by ref
    parlay::sequence<center<double>> centers2(k,center<double>()); 
    parlay::sequence<center<double>> centers3(k,center<double>());
    for (int i = 0; i < k; i++) {
        centers2[i].id = centers[i].id;
        centers2[i].coordinates=sequence<double>(d);
        centers3[i].id = centers[i].id;
        centers3[i].coordinates=sequence<double>(d);
        for (int j = 0; j < d; j++) {
            centers2[i].coordinates[j] = static_cast<double>(centers[i].coordinates[j]);
            centers3[i].coordinates[j] = static_cast<double>(centers[i].coordinates[j]);

        }
    }

    if (DEBUG_main) {
         std::cout << "centers: " << std::endl;
    for (int i = 0; i < k; i++) {
       print_center(centers[i]);
       print_center(centers2[i]);
        
    }

    }
   
    
    double epsilon = 0.01;

    std::cout << "started the first kmeans\n";;

    //double runtime1 = 73.2417;
    double runtime1 = kmeans_double_center<T>(v, centers2,k, m,epsilon).second;
    std::cout << "finished the first kmeans " << runtime1 << std::endl;
    
    if (DEBUG_main) {
        std::cout << "centers: " << std::endl;
        for (int i = 0; i < k; i++) {
            std::cout << "i:\n";
            print_center(centers[i]);
            print_center(centers2[i]);
        }

        parlay::sequence<size_t> belonging(k);
        for (int i = 0; i < n; i++) {
            belonging[v[i].best] += 1;
        }
        print_seq(belonging);
        assert(std::accumulate(belonging)==v.size());

    }
    
    return;
}


//bench without a pre-conversion of the points to doubles
template <typename T>
void bench_vd(parlay::sequence<point<T>> &v, size_t k, size_t m, double &runtime, bool DEBUG_main){
    assert(v.size() > 0); //want a nonempty point set
    size_t n = v.size();
    size_t d = v[0].coordinates.size();
    std::cout << "d: " << d << std::endl;
    std::cout << "calling vd" << std::endl;

    parlay::sequence<center<T>> centers = create_centers(v,n,k,d);
    if (DEBUG_main) {
        print_point(v[0]);
        print_point(v[1]);

    }
    
    //need to properly copy the centers because we are passing by ref
    parlay::sequence<center<float>> centers2(k,center<float>()); 
    parlay::sequence<center<double>> centers3(k,center<double>());
    for (int i = 0; i < k; i++) {
        centers2[i].id = centers[i].id;
        centers2[i].coordinates=sequence<float>(d);
        centers3[i].id = centers[i].id;
        centers3[i].coordinates=sequence<double>(d);
        for (int j = 0; j < d; j++) {
            centers2[i].coordinates[j] = static_cast<float>(centers[i].coordinates[j]);
            centers3[i].coordinates[j] = static_cast<double>(centers[i].coordinates[j]);

        }
    }

    if (DEBUG_main) {
         std::cout << "centers: " << std::endl;
    for (int i = 0; i < k; i++) {
       print_center(centers[i]);
       print_center(centers2[i]);
        
    }

    }
   
    
    double epsilon = 0.01;

    std::cout << "started the first kmeans\n";;

    //double runtime1 = 73.2417;
    double runtime1 = kmeans_vd<T>(v, centers2,k, m,epsilon);
    std::cout << "finished the first kmeans " << runtime1 << std::endl;
    double runtime2 = kmeans_double_center<T>(v,centers3,k,m,epsilon).second;
    std::cout << "finished the second kmeans " << runtime2 << std::endl;

    if (DEBUG_main) {
        std::cout << "centers: " << std::endl;
    for (int i = 0; i < k; i++) {
        std::cout << "i:\n";
        print_center(centers2[i]);
        print_center(centers3[i]);
    }

    parlay::sequence<size_t> belonging(k);
    for (int i = 0; i < n; i++) {
        belonging[v[i].best] += 1;
    }
    std::cout << "printing belonging: " << std::endl;
    print_seq(belonging);
    std::cout << "finished printing belonging " << std::endl;
    assert(std::accumulate(belonging)==v.size());

    }
    
    return;
}

//./kmeans_integrated -k 10 -i ../Data/base.1B.u8bin.crop_nb_1000000 -f bin -t uint8 -m 10
//./kmeans_integrated -k 10 -i ../Data/base.1B.u8bin-16.crop_nb_1000 -f bin -t uint8 -m 10 -c original

//run with this 
int main(int argc, char* argv[]){
    //P is the prompt
    commandLine P(argc, argv, "[-k <n_clusters>] [-m <iterations>] [-o <output>] [-i <input>] [-f <ft>] [-t <tp>] [-d <DEBUG>]");

    size_t k = P.getOptionLongValue("-k", 10); //k is number of clusters
    size_t max_iterations = P.getOptionLongValue("-m", 1000); //max_iterations is the max # of Lloyd iters kmeans will run
    std::string output = std::string(P.getOptionValue("-o", "kmeans_results.csv")); //maybe the kmeans results get written into this csv
    std::string input = std::string(P.getOptionValue("-i", "")); //the input file
    std::string ft = std::string(P.getOptionValue("-f", "bin")); //file type, bin or vecs
    std::string tp = std::string(P.getOptionValue("-t", "uint8")); //data type
    std::string debug = std::string(P.getOptionValue("-d", "false"));
    std::string impl = std::string(P.getOptionValue("-c", "none"));

    double runtime;

    bool DEBUG_main=false;
    if (debug=="true") {
        DEBUG_main=true;
    }
    
    if(input == ""){ //if no input file given, quit
        std::cout << "Error: input file not specified" << std::endl;
        abort();
    }

    if((ft != "bin") && (ft != "vec")){ //if the file type chosen is not one of the two approved file types, quit 
    std::cout << "Error: file type not specified correctly, specify bin or vec" << std::endl;
    abort();
    }

    if((tp != "uint8") && (tp != "int8") && (tp != "float")){ //if the data type isn't one of the three approved data types, quit
        std::cout << "Error: vector type not specified correctly, specify int8, uint8, or float" << std::endl;
        abort();
    }

    if((ft == "vec") && (tp == "int8")){ //you can't store int8s in a vec file apparently I guess
        std::cout << "Error: incompatible file and vector types" << std::endl;
        abort();
    }


    if (ft == "vec") {
        if (tp == "float") { //if the file type is vec and the data type is float, use the parse_fvecs function to get out the points
            parlay::sequence<point<float>> v = parse_fvecs(input.c_str());
            if (impl == "original") {
                bench_original<float>(v, k, max_iterations, runtime,DEBUG_main); 
            }
            else if (impl=="euc_dist") {
                bench_euc_dist_version<float>(v,k,max_iterations,runtime,DEBUG_main);
            }
            else if (impl=="doubled_centers") {
                bench_no_double_conversion<float>(v, k, max_iterations, runtime,DEBUG_main); //call kmeans

            }
            else {
                std::cout << "Please provide an implementation choice" << std::endl;
                abort();
            }
        } 
        else if (tp == "uint8") {
            auto v = parse_bvecs(input.c_str());
             if (impl == "original") {
                bench_original<uint8_t>(v, k, max_iterations, runtime,DEBUG_main); 
            }
            else if (impl=="euc_dist") {
                bench_euc_dist_version<uint8_t>(v,k,max_iterations,runtime,DEBUG_main);
            }
            else if (impl=="doubled_centers") {
                bench_no_double_conversion<uint8_t>(v, k, max_iterations, runtime,DEBUG_main); //call kmeans

            }
            else {
                std::cout << "Please provide an implementation choice" << std::endl;
                abort();
            }
        } else {
            std::cout << "Error: vector type can only be float or uint8. Supplied type is " << tp << "." << std::endl;
            abort();
        }
    } 
    else if (ft == "bin"){
        std::cout << "bin input\n";
        if (tp == "float") {
            auto v = parse_fbin(input.c_str());
            if (impl == "original") {
                bench_original<float>(v, k, max_iterations, runtime,DEBUG_main); 
            }
            else if (impl=="euc_dist") {
                bench_euc_dist_version<float>(v,k,max_iterations,runtime,DEBUG_main);
            }
            else if (impl=="doubled_centers") {
                bench_no_double_conversion<float>(v, k, max_iterations, runtime,DEBUG_main); //call kmeans

            }
            else {
                std::cout << "Please provide an implementation choice" << std::endl;
                abort();
            }
        } else if (tp == "uint8") {
            auto v = parse_uint8bin(input.c_str());
            std::cout << "going for a uint8 bin input" << std::endl;
             if (impl == "original") {
                bench_original<uint8_t>(v, k, max_iterations, runtime,DEBUG_main); 
            }
            else if (impl=="euc_dist") {
                bench_euc_dist_version<uint8_t>(v,k,max_iterations,runtime,DEBUG_main);
            }
            else if (impl=="doubled_centers") {
                bench_no_double_conversion<uint8_t>(v, k, max_iterations, runtime,DEBUG_main); //call kmeans

            }
            else if (impl == "vd") {
                bench_vd(v,k,max_iterations,runtime,DEBUG_main);
            }
            else {
                std::cout << "Please provide an implementation choice" << std::endl;
                abort();
            }
        } 


        } 
        else if (tp == "int8") {
            auto v = parse_int8bin(input.c_str());

             if (impl == "original") {
                bench_original<int8_t>(v, k, max_iterations, runtime,DEBUG_main); 
            }
            else if (impl=="euc_dist") {
                bench_euc_dist_version<int8_t>(v,k,max_iterations,runtime,DEBUG_main);
            }
            else if (impl=="doubled_centers") {
                bench_no_double_conversion<int8_t>(v, k, max_iterations, runtime,DEBUG_main); //call kmeans

            }
            else {
                std::cout << "Please provide an implementation choice" << std::endl;
                abort();
            }
   
        } else {
            // this should actually be unreachable
            std::cout << "Error: bin type can only be float, uint8, or int8. Supplied type is " << tp << "." << std::endl;
            abort();
        }
    
    
    std::cout << "Done in " << runtime << std::endl;

    return 0;
}