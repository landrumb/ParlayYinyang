#include "types.h"
#include "parse_files.h"
#include "parse_command_line.h"

#include "impshared_4.h"

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
    parlay::sequence<center<T>> centers(k);
    for (size_t i = 0; i < k; i++) {
        centers[i].coordinates = parlay::sequence<T>(v[0].coordinates.size());
    }
    runtime = naive_kmeans<T>(v, k, m);

   
    return;
}


//./kmeans -k 10 -i ../base.1B.u8bin.crop_nb_1000000 -f bin -t uint8 -m 10
//run with this 
int main(int argc, char* argv[]){
    //P is the prompt
    commandLine P(argc, argv, "[-k <n_clusters>] [-m <iterations>] [-o <output>] [-i <input>] [-f <ft>] [-t <tp>]");

    size_t k = P.getOptionLongValue("-k", 10); //k is number of clusters
    size_t max_iterations = P.getOptionLongValue("-m", 1000); //max_iterations is the max # of Lloyd iters kmeans will run
    std::string output = std::string(P.getOptionValue("-o", "kmeans_results.csv")); //maybe the kmeans results get written into this csv
    std::string input = std::string(P.getOptionValue("-i", "")); //the input file
    std::string ft = std::string(P.getOptionValue("-f", "bin")); //file type, bin or vecs
    std::string tp = std::string(P.getOptionValue("-t", "uint8")); //data type

    double runtime;
    
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
            bench<float>(v, k, max_iterations, runtime); //call kmeans
        } 
        else if (tp == "uint8") {
            auto v = parse_bvecs(input.c_str());
            bench<uint8_t>(v, k, max_iterations, runtime);
        } else {
            std::cout << "Error: vector type can only be float or uint8. Supplied type is " << tp << "." << std::endl;
            abort();
        }
    } 
    else if (ft == "bin"){
        if (tp == "float") {
            auto v = parse_fbin(input.c_str());
            bench<float>(v, k, max_iterations, runtime);
        } else if (tp == "uint8") {
            auto v = parse_uint8bin(input.c_str());
            bench<uint8_t>(v, k, max_iterations, runtime);
        } else if (tp == "int8") {
            auto v = parse_int8bin(input.c_str());
            parlay::sequence<center<int8_t>> centers(k);
            bench<int8_t>(v, k, max_iterations, runtime);
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