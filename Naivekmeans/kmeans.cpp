#include "types.h"
#include "parse_files.h"
#include "parse_command_line.h"

//#include "impshared_4.h"
#include "impshared_5.h"

#include "kmeans_guy.h"

#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <set>
#include <atomic>
#include <utility>
#include <type_traits>

//print out the average center size, number of empty centers from a run
//runtime stored in a reference so escapes the function
//why is this inline? Q*
template <typename T>
inline void bench(parlay::sequence<point<T>> &v, size_t k, size_t m, double &runtime){
   
    double epsilon = 0.01;
    
    runtime = kmeans_justtime<T>(v, k, m,epsilon);

   
    return;
}

//bench without a pre-conversion of the points to doubles
template <typename T>
void bench_no_double_conversion(parlay::sequence<point<T>> &v, size_t k, size_t m, double &runtime, bool DEBUG_main){
    assert(v.size() > 0); //want a nonempty point set
    size_t n = v.size();
    size_t d = v[0].coordinates.size();
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

    // std::cout << "points: " << std::endl;
    // for (int i = 0; i < n; i++) {
    //     print_point(v[i]);
    // }
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
    double runtime1 = kmeans_doublecenter<T>(v, centers2,k, m,epsilon).second;
    std::cout << "finished the first kmeans " << runtime1 << std::endl;
    // double runtime2 = guy_kmeans<double>(v2,centers3,k,m,epsilon).second;
    // std::cout << "guy: " << runtime2 << ", us: " << runtime1 << std::endl;

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

template <typename T>
void bench_givecenters(parlay::sequence<point<T>> &v, size_t k, size_t m, double &runtime, bool DEBUG_main){
    assert(v.size() > 0); //want a nonempty point set
    size_t n = v.size();
    size_t d = v[0].coordinates.size();
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

    // std::cout << "points: " << std::endl;
    // for (int i = 0; i < n; i++) {
    //     print_point(v[i]);
    // }
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
//./kmeans -k 10 -i ../Data/base.1B.u8bin.crop_nb_1000000 -f bin -t uint8 -m 10
//./kmeans -k 10 -i ../Data/base.1B.u8bin-16.crop_nb_1000 -f bin -t uint8 -m 10 

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
            bench_givecenters<float>(v, k, max_iterations, runtime,DEBUG_main); //call kmeans
        } 
        else if (tp == "uint8") {
            auto v = parse_bvecs(input.c_str());
            bench_givecenters<uint8_t>(v, k, max_iterations, runtime,DEBUG_main);
        } else {
            std::cout << "Error: vector type can only be float or uint8. Supplied type is " << tp << "." << std::endl;
            abort();
        }
    } 
    else if (ft == "bin"){
        std::cout << "bin input\n";
        if (tp == "float") {
            auto v = parse_fbin(input.c_str());
            bench_givecenters<float>(v, k, max_iterations, runtime,DEBUG_main);
        } else if (tp == "uint8") {
            auto v = parse_uint8bin(input.c_str());
            std::cout << "going for a uint8 bin input" << std::endl;
            bench_no_double_conversion<uint8_t>(v, k, max_iterations, runtime,DEBUG_main);
        } else if (tp == "int8") {
            auto v = parse_int8bin(input.c_str());
            parlay::sequence<center<int8_t>> centers(k);
            bench_givecenters<int8_t>(v, k, max_iterations, runtime,DEBUG_main);
        } else {
            // this should actually be unreachable
            std::cout << "Error: bin type can only be float, uint8, or int8. Supplied type is " << tp << "." << std::endl;
            abort();
        }
    }

    //bs he didn't make the output file coordination yet 

    std::cout << "Done in " << runtime << std::endl;

    return 0;
}