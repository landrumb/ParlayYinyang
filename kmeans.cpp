#include "parse_command_line.h"
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
#include <type_traits>

template <typename T>
int empty_centers(parlay::sequence<center<T>> &centers){
    int empty = 0;
    for (size_t i = 0; i < centers.size(); i++) {
        if (centers[i].points.size() == 0) {
            empty++;
        }
    }
    return empty;
}

template<typename T>
double avg_center_size(parlay::sequence<center<T>> &centers){
    double avg = 0;
    for (size_t i = 0; i < centers.size(); i++) {
        avg += centers[i].points.size();
    }
    return avg / centers.size();
}

// the world's most heinous function name
template <typename T>
double dist_between_first_center_and_centroid(parlay::sequence<point<T>> &v, parlay::sequence<center<T>> &centers, Distance D){
    size_t dim = centers[0].coordinates.size();
    parlay::sequence<double> centroid(dim);
    for (size_t i = 0; i < dim; i++) {
        centroid[i] = 0;
    }

    for (auto it = centers[0].points.begin(); it != centers[0].points.end(); it++) {
        for (size_t i = 0; i < dim; i++) {
            centroid[i] += v[*it].coordinates[i];
        }
    }
    
    for (size_t i = 0; i < dim; i++) {
        centroid[i] /= (double) centers[0].points.size();
    }

    parlay::sequence<T> centroid_seq(centroid.begin(), centroid.end());

    return D.distance(centers[0].coordinates.begin(), centroid_seq.begin(), dim);
}

template <typename T>
inline void bench(parlay::sequence<point<T>> &v, size_t k, size_t m, Distance *D, double &runtime){
    parlay::sequence<center<T>> centers(k);
    for (size_t i = 0; i < k; i++) {
        centers[i].coordinates = parlay::sequence<T>(v[0].coordinates.size());
    }
    runtime = kmeans<T>(v, centers, k, m, *D);

    std::cout << empty_centers(centers) << " empty centers" << std::endl;
    std::cout << avg_center_size(centers) << " average center size" << std::endl;
    std::cout << dist_between_first_center_and_centroid(v, centers, *D) << " distance between first center and centroid" << std::endl;
    return;
}


int main(int argc, char* argv[]){
    commandLine P(argc, argv, "[-k <n_clusters>] [-m <iterations>] [-o <output>] [-i <input>] [-f <ft>] [-t <tp>] [-D <dist>]");

    size_t k = P.getOptionLongValue("-k", 10);
    size_t max_iterations = P.getOptionLongValue("-m", 1000);
    std::string output = std::string(P.getOptionValue("-o", "kmeans_results.csv"));
    std::string input = std::string(P.getOptionValue("-i", ""));
    std::string ft = std::string(P.getOptionValue("-f", "bin"));
    std::string tp = std::string(P.getOptionValue("-t", "uint8"));
    std::string dist = std::string(P.getOptionValue("-D", "Euclidian"));

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
            bench<float>(v, k, max_iterations, D, runtime);
        } 
        else if (tp == "uint8") {
            auto v = parse_bvecs(input.c_str());
            bench<uint8_t>(v, k, max_iterations, D, runtime);
        } else {
            std::cout << "Error: vector type can only be float or uint8. Supplied type is " << tp << "." << std::endl;
            abort();
        }
    } 
    else if (ft == "bin"){
        if (tp == "float") {
            auto v = parse_fbin(input.c_str());
            bench<float>(v, k, max_iterations, D, runtime);
        } else if (tp == "uint8") {
            auto v = parse_uint8bin(input.c_str());
            bench<uint8_t>(v, k, max_iterations, D, runtime);
        } else if (tp == "int8") {
            auto v = parse_int8bin(input.c_str());
            parlay::sequence<center<int8_t>> centers(k);
            bench<int8_t>(v, k, max_iterations, D, runtime);
        } else {
            // this should actually be unreachable
            std::cout << "Error: bin type can only be float, uint8, or int8. Supplied type is " << tp << "." << std::endl;
            abort();
        }
    }

    std::cout << "Done in " << runtime << std::endl;

    return 0;
}