#include <iostream>
#include <cstdlib>
#include <cmath>

#include "krnl_matrix.h"



// Simple reference implementation: C = A * B
void reference_mm(const int A[SIZE][SIZE],
                  const int B[SIZE][SIZE],
                  int C[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            C[i][j] = 0;
            for (int k = 0; k < SIZE; ++k)
                C[i][j] += A[i][k] * B[k][j];
        }
    }
}

int main() {
constexpr unsigned tests = 8;    

    static int A[tests][SIZE][SIZE];
    static int B[tests][SIZE][SIZE];
    static int C[tests][SIZE][SIZE];
    static int Ref[tests][SIZE][SIZE];
    // Initialize random
    std::srand(600);

    // Generate random matrices
    for (int n = 0; n < tests; ++n)
        for (int i = 0; i < SIZE; ++i) {
            for (int j = 0; j < SIZE; ++j) {
                A[n][i][j] = (std::rand() % 1000) - 499;
                B[n][i][j] = (std::rand() % 1000) - 499;
                C[n][i][j] = 0;
            }
        }
    

    std::cout << "Computing reference result..." << std::endl;
    for (int n = 0; n < tests; ++n)
        reference_mm(A[n], B[n], Ref[n]);

    std::cout << "Running krnl_matrix..." << std::endl;
    for (int n = 0; n < tests; ++n)
        krnl_matrix(A[n], B[n], C[n]);

    // Compare the results
    int mismatches = 0;
    for (int n = 0; n < tests; ++n)
        for (int i = 0; i < SIZE; ++i) {
            for (int j = 0; j < SIZE; ++j) {
                if (C[n][i][j] != Ref[n][i][j]) {
                    if (mismatches < 50) {
                        std::cout << "Mismatch at (" << i << "," << j << ")" << "in test " << n << ": "
                                  << C[n][i][j] << " != " << Ref[n][i][j] << std::endl;
                    }
                    ++mismatches;
                }
            }
        }

    if (mismatches == 0)
        std::cout << "Test PASSED! All values match." << std::endl;
    else
        std::cout << "Test FAILED: " << mismatches << " mismatches found." << std::endl;

    return -(mismatches != 0);
}
