#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include "krnl_sobel.h"

void p2mm(
    gray_t* img,
    gray_stream& in)
{
#pragma HLS INTERFACE m_axi     port=img  offset=slave bundle=gmem
#pragma HLS INTERFACE axis      port=in
#pragma HLS INTERFACE s_axilite port=img  bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    // Read a full frame from the stream and write sequentially to memory.
    for (int y = 0; y < IMG_HEIGHT; ++y) {
        for (int x = 0; x < IMG_WIDTH; ++x) {
#pragma HLS PIPELINE II=1
            gray_axis pkt = in.read();
            img[y * IMG_WIDTH + x] = pkt.data;
        }
    }
}