#pragma once

#include "hls_task.h"
#include <ap_int.h>
#include "ap_axi_sdata.h"

#define IMG_WIDTH 512
#define IMG_HEIGHT 512

typedef ap_uint<24> pixel_t;
typedef ap_int<11> gradient_t;
typedef ap_uint<8> gray_t;
typedef hls::axis<pixel_t, 1, 0, 0, AXIS_ENABLE_LAST | AXIS_ENABLE_USER> pixel_axis;
typedef hls::axis<gray_t, 1, 0, 0, AXIS_ENABLE_LAST | AXIS_ENABLE_USER> gray_axis;
typedef struct { // hls::axis and other ap_axi_sdata types are only allowed for interfacing in/out of the kernel, not internally
    gradient_t data;
    bool user;
    bool last;
} gradient_axis;
typedef hls::stream<gradient_axis> gradient_stream;
typedef hls::stream<pixel_axis> pixel_stream;
typedef hls::stream<gray_axis> gray_stream;

void krnl_sobel(pixel_stream& in, gray_stream& out);