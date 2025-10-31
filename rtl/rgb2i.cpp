
#include <hls_stream.h>

void rgb2i(hls::stream<ap_uint<24>> rgb_stream, hls::stream<ap_uint<8>> i_stream) {
    const ap_uint<24> rgb = rgb_stream.read();
    const ap_uint<8> r = rgb(7, 0);
    const ap_uint<8> g = rgb(15, 8);
    const ap_uint<8> b = rgb(23, 16);
    i_stream.write((306*r + 601*g + 117*b) >> 10);
}