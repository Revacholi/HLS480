#include "krnl_matrix.h"

void krnl_matrix(int A[SIZE][SIZE], int B[SIZE][SIZE], int C[SIZE][SIZE]) {

    /** 
        All arrays (A, B, C) are accessed through the same AXI memory interface by default. 
        We assign independent memory interfaces (bundles) to A, B, and C, 
        so they can access each other in parallel without interference.

        m_axi: Indicates accessing external memory using the AXI Master interface
        bundle=gmem0/gmem1/gmem2: Creates three independent physical interfaces (global mem)
        offset=slave: The memory address offset is provided by the CPU's control registers.
        Simply put, the CPU tells the FPGA "where the data is in memory," and the FPGA accesses it based on this address. 
        This is standard practice because the FPGA doesn't know the memory addresses allocated by the operating system. 
    **/
    #pragma HLS INTERFACE m_axi port=A offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=B offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=C offset=slave bundle=gmem2

    /**
        Performance bottleneck: Repetitive memory accesses.
        Solution: create local buffers to read A and B into the FPGA's on-chip BRAM all at once
    **/
    int local_A[SIZE][SIZE];
    int local_B[SIZE][SIZE];    

    localA_i_loop: for (int i = 0; i < SIZE; i++) {
        localA_j_loop: for (int j = 0; j < SIZE; j++) {
            #pragma HLS PIPELINE II=2
            local_A[i][j] = A[i][j];
        }
    }
    
    localB_i_loop: for (int i = 0; i < SIZE; i++) {
        localB_j_loop: for (int j = 0; j < SIZE; j++) {
            #pragma HLS PIPELINE II=2
            local_B[i][j] = B[i][j];
        }
    }


    i_loop: for (int i = 0; i < SIZE; ++i) {
        j_loop: for (int j = 0; j < SIZE; ++j) {
            int c = 0;           
            k_loop: for (int k = 0; k < SIZE; ++k) {
                #pragma HLS PIPELINE II=1
                #pragma HLS UNROLL factor=16
                // Enable pipeline, II=1,Initiation(new iter) Interval = 1 clk cycle
                c = c + local_A[i][k] * local_B[k][j];
            }
            C[i][j] = c;
        } 
    }
}

