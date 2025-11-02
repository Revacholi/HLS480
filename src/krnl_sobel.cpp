#include "krnl_sobel.h"
#include "hls_math.h"

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

void conv_3x3(const int conv_kernel[3][3], gradient_stream& in, gradient_stream& out) {
    #pragma HLS FUNCTION_INSTANTIATE variable=conv_kernel
    
    /* Your implementation here*/
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

    ap_int<12> v = grad.data;
    ap_uint<12> abs_v = hls::abs(v);
    ap_uint<9> small = (ap_uint<9>)(abs_v >> 3);
    tmp.data = (128 + (small*(-hls::signbit(v)))).range(7, 0);
    tmp.last = grad.last;
    tmp.user = grad.user;
    out.write(tmp);
}

void krnl_sobel(pixel_stream& in, gray_stream& out) {
    #pragma HLS INTERFACE axis port=in
    #pragma HLS INTERFACE axis port=out
    #pragma HLS INTERFACE ap_ctrl_none port=return

    gradient_stream intensity_stream("intensity_stream");
    gradient_stream sobel_tmp1_stream("sobel_tmp1_stream");
    gradient_stream sobel_tmp2_stream("sobel_tmp2_stream");

    hls_thread_local hls::task t_convert(rgb2i, in, intensity_stream);
    hls_thread_local hls::task t_conv1(conv_3x3, Gx, intensity_stream, sobel_tmp1_stream);
    hls_thread_local hls::task t_conv2(conv_3x3, Gy, sobel_tmp1_stream, sobel_tmp2_stream);
    hls_thread_local hls::task t_downscale(downscale, sobel_tmp2_stream, out);

}
