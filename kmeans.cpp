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


int main(int argc, char* argv[]){
    commandLine P(argc, argv, "[-k <n_clusters>] [-m <iterations>] [-d <dimension>] [-o <output>] [-i <input>] [-f <ft>] [-t <tp>]");

    size_t k = P.getOptionLongValue("-k", 10);
    size_t max_iterations = P.getOptionLongValue("-m", 10);
    size_t dim = P.getOptionLongValue("-d", 2);
    std::string output = P.getOptionValue("-o");
    std::string input = P.getOptionValue("-i");
    std::string ft = P.getOptionValue("-f");
    std::string tp = P.getOptionValue("-t");
    
    f((ft != "bin") && (ft != "vec")){
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

    Distance* D = new EuclideanDistance();

    


}