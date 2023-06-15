// #include "parse_command_line.h"
#include "types.h"
#include "parlay/parallel.h"
#include "parlay/primitives.h"
#include "parlay/sequence.h"
#include "parlay/slice.h"
#include "parlay/random.h"
#include "parlay/internal/get_time.h"
#include "NSGDist.h"
#include "parse_files.h"

#include "implementations_ben.h"

#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <set>
#include <atomic>
#include <utility>


int main(int argc, char* argv[]){
    // commandLine P(argc, argv, "[-k <n_clusters>] [-m <iterations>] [-d <dimension>] [-o <output>] [-i <input>] [-f <ft>] [-t <tp>] [-D <dist>]");

    // size_t k = P.getOptionLongValue("-k", 10);
    // size_t max_iterations = P.getOptionLongValue("-m", 1000);
    // size_t dim = P.getOptionLongValue("-d", 2);
    // std::string output = std::string(P.getOptionValue("-o", "kmeans_results.csv"));
    // std::string input = std::string(P.getOptionValue("-i", ""));
    // std::string ft = std::string(P.getOptionValue("-f", "bin"));
    // std::string tp = std::string(P.getOptionValue("-t", "uint8"));
    // std::string dist = std::string(P.getOptionValue("-D", "Euclidian"));

    size_t k = 10;
    size_t max_iterations = 1000;
    size_t dim = 2;
    std::string output = "kmeans_results.csv";
    std::string input = "";
    std::string ft = "bin";
    std::string tp = "uint8";
    std::string dist = "Euclidian";


    double runtime;
    
    if(input == ""){
        std::cout << "Error: input file not specified" << std::endl;
        abort();
    }

    if((ft != "bin") && (ft != "vec")){
    std::cout << "Error: file type not specified correctly, specify bin or vec" << std::endl;
    abort();
    }

    if((tp != "uint8") && (tp != "int8") && (tp != "float")){
        std::cout << "Error: vector type not specified correctly, specify int8, uint8, or float" << std::endl;
        abort();
    }

    if((ft == "vec") && (tp == "int8")){
        std::cout << "Error: incompatible file and vector types" << std::endl;
        abort();
    }

    Distance* D;
    if (dist == "Euclidian") {
        std::cout << "Using Euclidian distance" << std::endl;
        D = new Euclidian_Distance();
    } else if (dist == "mips") {
        std::cout << "Using MIPS distance" << std::endl;
        D = new Mips_Distance();
    } else {
        std::cout << "Error: distance type not specified correctly, specify Euclidean or mips" << std::endl;
        abort();
    }

    if (ft == "vec") {
        if (tp == "float") {
            parlay::sequence<point<float>> v = parse_fvecs(input.c_str());
            parlay::sequence<center<float>> centers(k);
            auto output_pair = naive_kmeans<float>(v, centers, k, max_iterations, D);
        } 
        // else if (tp == "uint8") {
        //     auto v = parse_bvecs(input.c_str());
        //     parlay::sequence<center<uint8_t>> centers;
        //     auto [centers, runtime] = naive_kmeans<uint8_t>(v, k, max_iterations, D);
        // } else {
        //     std::cout << "Error: vector type can only be float or uint8. Supplied type is " << tp << "." << std::endl;
        //     abort();
        // }
    } 
    // else if (ft == "bin"){
    //     if (tp == "float") {
    //         auto v = parse_fbin(input.c_str());
    //         parlay::sequence<center<float>> centers;
    //         auto [centers, runtime] = naive_kmeans<float>(v, k, max_iterations, D);
    //     } else if (tp == "uint8") {
    //         auto v = parse_uint8bin(input.c_str());
    //         parlay::sequence<center<uint8_t>> centers;
    //         auto [centers, runtime] = naive_kmeans<uint8_t>(v, k, max_iterations, D);
    //     } else if (tp == "int8") {
    //         auto v = parse_int8bin(input.c_str());
    //         parlay::sequence<center<int8_t>> centers;
    //         auto [centers, runtime] = naive_kmeans<int8_t>(v, k, max_iterations, D);
    //     } else {
    //         // this should actually be unreachable
    //         std::cout << "Error: bin type can only be float, uint8, or int8. Supplied type is " << tp << "." << std::endl;
    //         abort();
    //     }
    // }

    // std::cout << "Done in " << runtime << std::endl;

    return 0;
}