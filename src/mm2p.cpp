
#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include "krnl_sobel.h"

void mm2p(
    ap_uint<8>* img,
    pixel_stream& out)
{
#pragma HLS INTERFACE m_axi     port=img  offset=slave bundle=gmem
#pragma HLS INTERFACE axis      port=out
#pragma HLS INTERFACE s_axilite port=img  bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    int y = 0;
    int x = 0;
    ap_uint<8> buffer[12];
    unsigned I = 0;
    while (y < IMG_HEIGHT) {
        #pragma HLS PIPELINE II=1 rewind
        for (unsigned i = 0; i < 12; i++) {
#pragma HLS unroll
            buffer[i] = img[I+i];
        }
        I += 12;

        for (unsigned i = 0; i < 4; i++) {
            #pragma HLS PIPELINE II=1 rewind
            pixel_t pix;
            pix(7,0)= buffer[i*3 + 0];
            pix(15,8)= buffer[i*3 + 1];
            pix(23,16)= buffer[i*3 + 2];
            pixel_axis pkt;
            pkt.data = pix;
            pkt.user = (y == 0 && x == 0) ? 1 : 0;         // SOF = first pixel of frame
            pkt.last = (x == (IMG_WIDTH - 1)) ? 1 : 0;         // EOL = last pixel of each line
            if (y < IMG_HEIGHT)
                out.write(pkt);

            if (x == (IMG_WIDTH - 1)) {
                x = 0;
                y++;
            } else {
                x++;
            }
        }
    }
}
