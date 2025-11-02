#pragma once

#include "hls_tasks.h"
#include <ap_int.h>

#define IMG_WIDTH 512
#define IMG_HEIGHT 512

typedef ap_uint<24> pixel_t;
typedef ap_int<11> gradient_t;
typedef ap_uint<8> gray_t;
typedef hls::axis<pixel_t, 1, 0, 0, AXIS_ENABLE_LAST | AXIS_ENABLE_USER> pixel_axis;
typedef hls::axis<gray_t, 1, 0, 0, AXIS_ENABLE_LAST | AXIS_ENABLE_USER> gray_axis;
typedef hls::axis<gradient_t, 1, 0, 0, AXIS_ENABLE_LAST | AXIS_ENABLE_USER> gradient_axis;
typedef hls::stream<gradient_axis> gradient_stream;
typedef hls::stream<pixel_axis> pixel_stream;
typedef hls::stream<gray_axis> gray_stream;

void krnl_sobel(pixel_stream& in, gray_stream& out);