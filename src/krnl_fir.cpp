
#include "krnl_fir.h"

void krnl_fir(int& x, int& y) {
#pragma HLS pipeline
#pragma HLS INTERFACE axis port=x
#pragma HLS INTERFACE axis port=y
#pragma HLS INTERFACE ap_ctrl_none port=return
    static int delay[TAPS] = {0};

    int sum = 0;
    for (int i = TAPS-1; i > 0; --i) {
        delay[i] = delay[i-1];
    }
    delay[0] = x;
    for (int i = 0; i < TAPS; ++i) {
        sum += delay[i] * coeffs[i];
    }
    y= sum;
}