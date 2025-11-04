#include "krnl_sobel.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <cassert>

#ifdef USE_OPENCV
#include <opencv2/opencv.hpp>
#endif

using namespace std;

static bool load_image(const string &fname, vector<uint8_t> &out_rgb, unsigned &w, unsigned &h) {
#ifdef USE_OPENCV
    cv::Mat img = cv::imread(fname, cv::IMREAD_COLOR);
    if (img.empty()) return false;
    cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
    w = img.cols; h = img.rows;
    out_rgb.resize(w*h*3);
    memcpy(out_rgb.data(), img.data, out_rgb.size());
    return true;
#else
    // simple PPM (P6) loader
    ifstream ifs(fname, ios::binary);
    if (!ifs) return false;
    string magic;
    ifs >> magic; if (magic != "P6") return false;
    // skip comments
    char ch; ifs.get(ch);
    while (ifs.peek() == '#') { string line; getline(ifs, line); }
    ifs >> w >> h;
    int maxv; ifs >> maxv; ifs.get(ch);
    if (maxv != 255) return false;
    out_rgb.resize(w*h*3);
    ifs.read((char*)out_rgb.data(), out_rgb.size());
    return true;
#endif
}

static bool save_image_pgm(const string &fname, const vector<uint8_t> &gray, unsigned w, unsigned h) {
#ifdef USE_OPENCV
    cv::Mat out(h, w, CV_8UC1, (void*)gray.data());
    return cv::imwrite(fname, out);
#else
    ofstream ofs(fname, ios::binary);
    if (!ofs) return false;
    ofs << "P5\n" << w << " " << h << "\n255\n";
    ofs.write((const char*)gray.data(), gray.size());
    return true;
#endif
}

int main(int argc, char **argv) {
    const string in_file  = (argc > 1) ? argv[1] : "../../../../../../data/in0.ppm";
    const string out_file = (argc > 2) ? argv[2] : "out.pgm";

    vector<uint8_t> in_rgb;
    unsigned w=0, h=0;
    if (!load_image(in_file, in_rgb, w, h)) {
        cerr << "Failed to load input image: " << in_file << "\n";
        return 1;
    }

    if (w != IMG_WIDTH || h != IMG_HEIGHT) {
        cerr << "Input image size must be " << IMG_WIDTH << "x" << IMG_HEIGHT << " (got "
             << w << "x" << h << ")\n";
        return 1;
    }

    // Prepare input stream
    pixel_stream in_stream("tb_in");
    gray_stream out_stream("tb_out");

    // push pixels into stream (tuser = SOF for first pixel only, tlast = EOL)
    for (unsigned y = 0; y < IMG_HEIGHT; ++y) {
        for (unsigned x = 0; x < IMG_WIDTH; ++x) {
            const unsigned idx = (y*IMG_WIDTH + x)*3;
            const ap_uint<8> r = in_rgb[idx + 0];
            const ap_uint<8> g = in_rgb[idx + 1];
            const ap_uint<8> b = in_rgb[idx + 2];
            pixel_axis p;
            p.data = ( (ap_uint<24>)b << 16 ) | ( (ap_uint<24>)g << 8 ) | (ap_uint<24>)r;
            p.last = (x == IMG_WIDTH - 1) ? 1 : 0;
            p.user = (x == 0 && y == 0) ? 1 : 0; // SOF on first pixel
            in_stream.write(p);
        }
    }

    // Call kernel (C-simulation)
    krnl_sobel(in_stream, out_stream);

    // Read outputs
    vector<uint8_t> out_gray;
    out_gray.resize(IMG_WIDTH * IMG_HEIGHT);
    for (unsigned i = 0; i < IMG_WIDTH * IMG_HEIGHT; ++i) {
        if (out_stream.empty()) {
            cerr << "Output stream underflow at pixel " << i << "\n";
            return 1;
        }
        gray_axis g = out_stream.read();
        out_gray[i] = (uint8_t)g.data;
    }

    if (!save_image_pgm(out_file, out_gray, IMG_WIDTH, IMG_HEIGHT)) {
        cerr << "Failed to save output image\n";
        return 1;
    }

    cout << "Test PASSED! Output written to " << out_file << "\n";
    return 0;
}