# DAT480 HLS Tutorial
## This repo contains three tutorial exercises for getting started with Vitis HLS

A very useuful resource for this is the Vitis High-Level Synthesis User Guide (UG1399)
https://docs.amd.com/r/2023.2-English/ug1399-vitis-hls/HLS-Pragmas
Especially the chapters on pragmas and libraries are great to reference when developing for HLS.

Another resource is the free book "Parallel Programming for FPGAs"
https://raw.githubusercontent.com/KastnerRG/pp4fpgas/gh-pages/main.pdf



## 1. FIR-filter

#pragma HLS pipeline

#pragma HLS INTERFACE axis port=x
#pragma HLS INTERFACE axis port=y
#pragma HLS INTERFACE ap_ctrl_none port=return


## 2. Matrix multiplication



## 3. Sobel filter