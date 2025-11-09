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

    //out.write(in.read()); 

    // gradient_t line_buffer[2][IMG_WIDTH];
    // #pragma HLS ARRAY_PARTITION variable=line_buffer dim=1 complete
    
    // // 3x3 window
    // gradient_t window[3][3];
    // #pragma HLS ARRAY_PARTITION variable=window dim=0 complete
    
    // // initialization
    // for(int i = 0; i < 2; i++) {
    //     for(int j = 0; j < IMG_WIDTH; j++) {
    //         line_buffer[i][j] = 0;
    //     }
    // }
    // for(int i = 0; i < 3; i++) {
    //     for(int j = 0; j < 3; j++) {
    //         window[i][j] = 0;
    //     }
    // }
    
    // // deal with stream data with inf loop
    // while(true) {
    //     // whole frame
    //     for(int row = 0; row < IMG_HEIGHT; row++) {
    //         for(int col = 0; col < IMG_WIDTH; col++) {
    //             #pragma HLS pipeline II=1
                
    //             gradient_axis in_pixel = in.read();
    //             gradient_t current_pixel = in_pixel.data;
                
    //             // move window to left
    //             for(int i = 0; i < 3; i++) {
    //                 window[i][0] = window[i][1];
    //                 window[i][1] = window[i][2];
    //             }
                
    //             // fill the right col
    //             window[0][2] = line_buffer[0][col];  
    //             window[1][2] = line_buffer[1][col];  
    //             window[2][2] = current_pixel;         
                
    //             // update line buffer
    //             line_buffer[0][col] = line_buffer[1][col];
    //             line_buffer[1][col] = current_pixel;
                
    //             // cal conv
    //             ap_int<20> sum = 0;
    //             for(int i = 0; i < 3; i++) {
    //                 for(int j = 0; j < 3; j++) {
 
    //                     int img_row = row - 1 + i;  
    //                     int img_col = col - 1 + j;
                        
    //                     // edge check
    //                     gradient_t pixel_val = 0;
    //                     if(img_row >= 0 && img_row < IMG_HEIGHT && 
    //                        img_col >= 0 && img_col < IMG_WIDTH) {
    //                         pixel_val = window[i][j];
    //                     }
                        
    //                     sum += pixel_val * conv_kernel[i][j];
    //                 }
    //             }
                
    //             gradient_axis out_pixel;
    //             out_pixel.data = (gradient_t)sum;
    //             out_pixel.user = in_pixel.user;  
    //             out_pixel.last = in_pixel.last;  
    //             out.write(out_pixel);
    //         }
    //     }
    // }
    
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
