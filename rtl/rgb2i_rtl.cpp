
#include <hls_stream.h>
ap_uint<8> rgb2i_rtl(const ap_uint<24> rgb) {
    const ap_uint<8> r = rgb(7, 0);
    const ap_uint<8> g = rgb(15, 8);
    const ap_uint<8> b = rgb(23, 16);
    return (306*r + 601*g + 117*b) >> 10;
}