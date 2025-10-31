#include <iostream>
#include <cstdlib>
#include <cmath>

#include "krnl_fir.h"
#define input_length 21

int main() {   
    int test[input_length] = {0, 1, 3, 7, 10, 7, 3, 1, 0, -1, -3, -7, -10, -7, -3, -1, 0, 1, 3, 7, 10};
    int ref[input_length] = {0,-1,-7,-19,-26,21,195,501,830,981,837,520,221,0,-221,-520,-837,-982,-837,-520,-221};

    // Compare the results
    int y;
    int mismatches = 0;
    for (int n = 0; n < input_length; ++n) {
        krnl_fir(test[n], y);
        mismatches += ref[n] != y;
        std::cout << ref[n] << " " << y << "\n";
    }

    if (mismatches == 0)
        std::cout << "Test PASSED! All values match." << std::endl;
    else
        std::cout << "Test FAILED: " << mismatches << " mismatches found." << std::endl;

    return -(mismatches != 0);
}

