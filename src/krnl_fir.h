
#pragma once

#include <vector>

#define TAPS 11
static int coeffs[TAPS] = {-1,  -4,  0,  12,  32,  46,  32,  12,  0,  -4,  -1};

void krnl_fir(int x, int& y);