
#include "krnl_matrix.h"

void krnl_matrix(int A[SIZE][SIZE], int B[SIZE][SIZE], int C[SIZE][SIZE]) {
    i_loop: for (int i = 0; i < SIZE; ++i) {
        j_loop: for (int j = 0; j < SIZE; ++j) {
            int c = 0;
            k_loop: for (int k = 0; k < SIZE; ++k) {
                c = c + A[i][k] * B[k][j];
            }
            C[i][j] = c;
        } 
    }
}