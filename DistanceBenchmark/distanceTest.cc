//Is this the correct way to call NSGDist?
//I am getting 37 and 93 as outputs for distance
//It seems to me that I should be getting (8-4)^2 + (3-1)^2 = 20 for Euclidian Distance

#include "NSGDist.h"
#include<vector>

int main() {
    
    for (int i = 0; i < 10; i++) {
        float* seq1 = new float[2];

        float* seq2 = new float[2];

        seq1[0] = 8;
        seq1[1] = 3;
        seq2[0] = 4;
        seq2[1] = 1;

        Distance* D = new Euclidian_Distance();
        
        std::cout << "nsg " << (D->distance(seq1, seq2, 2)) << std::endl;


        delete[] seq1;
        delete[] seq2;
    }


}