#include "krnl_sobel.h"
#include "hls_math.h"

#ifndef __SYNTHESIS__
#define MAT2P_SIM(m) (&(m[0][0]))
#else
#define MAT2P_SIM(m) (m)
#endif

static const int Gx[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};

static const int Gy[3][3] = {
    {1, 2, 1},
    {0, 0, 0},
    {-1, -2, -1}
};


void conv_3x3(gradient_stream& in, gradient_stream& out,
#ifdef __SYNTHESIS__
    const int conv_kernel[3][3]){
    #pragma HLS FUNCTION_INSTANTIATE variable=conv_kernel
    #pragma HLS inline off
#else
    // C-Simualtion does not support array paramter in functions to be used in hls::task
    const int* conv_kernel_p) {
    const int (*conv_kernel)[3] = reinterpret_cast<const int (*)[3]>(conv_kernel_p);
#endif
    
    /* Your implementation here*/
    out.write(in.read()); // Dummy implementation, remove this line when implementing the real convolution
    }

void rgb2i(pixel_stream& rgb_stream, gradient_stream& i_stream) {
    // Approximation of ITU-R BT.601 conversion
    #pragma HLS pipeline
    const pixel_axis rgb = rgb_stream.read();
    const ap_uint<8> r = rgb.data(7, 0);
    const ap_uint<8> g = rgb.data(15, 8);
    const ap_uint<8> b = rgb.data(23, 16);
    gradient_axis tmp;
    tmp.data = (306*r + 601*g + 117*b) >> 10;
    tmp.last = rgb.last;
    tmp.user = rgb.user;
    i_stream.write(tmp);
}

void downscale(gradient_stream& in, gray_stream& out) {
    #pragma HLS pipeline
    const gradient_axis grad = in.read();
    gray_axis tmp;

    auto v = grad.data + 128;
    if (v < 0) v = 0;
    if (v > 255) v = 255;
    tmp.data = (gray_t)(v(7,0));
    tmp.last = grad.last;
    tmp.user = grad.user;
    out.write(tmp);
}

void krnl_sobel(pixel_stream& in, gray_stream& out) {
    #pragma HLS INTERFACE axis port=in
    #pragma HLS INTERFACE axis port=out
    #pragma HLS INTERFACE ap_ctrl_none port=return

    hls_thread_local gradient_stream intensity_stream("intensity_stream");
    hls_thread_local gradient_stream sobel_tmp1_stream("sobel_tmp1_stream");
    hls_thread_local gradient_stream sobel_tmp2_stream("sobel_tmp2_stream");

    hls_thread_local hls::task t_convert(rgb2i, in, intensity_stream);
    hls_thread_local hls::task t_conv1(conv_3x3, intensity_stream, sobel_tmp1_stream, MAT2P_SIM(Gx));
    hls_thread_local hls::task t_conv2(conv_3x3, sobel_tmp1_stream, sobel_tmp2_stream, MAT2P_SIM(Gy));
    hls_thread_local hls::task t_downscale(downscale, sobel_tmp2_stream, out);

    #ifndef __SYNTHESIS__
    bool all_done = false;
    do {
        all_done = in.empty() &&
            intensity_stream.empty() &&
            sobel_tmp1_stream.empty() &&
            sobel_tmp2_stream.empty();
    } while (!all_done);
    #endif
}
