# DAT480 HLS Tutorial
### This repo contains three tutorial exercises for getting started with Vitis HLS



## 1. FIR-filter

First we are going to have a look at a basic HLS(High Level Synthesis) design, a FIR filter, and how we can configure it to get different implementation characteristics.

Start by generating and building the Vitis project:
```bash
make krnl_fir
```

This will create a Vitis HLS project in the folder `_x.xilinx_u55c_gen3x16_xdma_3_202210_1/` and build the `krnl_fir` kernel.

Now open the project in Vitis by running:
```bash
vitis -w _x.xilinx_u55c_gen3x16_xdma_3_202210_1
```
You should see `krnl_fir` under **VITIS COMPONENTS**, opening that, you should see the C++ files `krnl_fir.cpp` and `tb_fir.cpp` under *Sources* and *Test Bench*. These files contains the code for our kernel, and a test bench to run it. Have a look at both of these, we will be modifying them later.

Now look select `krnl_fir` under **FLOW**. The **FLOW** tab lists the actions that you can take on a specific components. For HLS components this is *C SIMULATION*, *C SYNTHESIS*, *C/RTL COSIMULATION*, *PACKAGE* and *IMPLEMENTATION*.
For now the first three are of interest for us.
- C Simulation runs the test bench, and executes the kernel as normal C++ code.
- C Synthesis compiles the kernel from C++ to RTL (Register Transfer Level) code, which is what is used to program the FPGA.
- C/RTL Co-simulation runs the test bench, but this time the RTL version of the kernel is executed on a digital logic simulator.

The Makefile that generated the project also ran C Synthesis, so we can have a look at the generated reports for this basic implementation of the FIR filter directly. Click on C Synthesis>Reports>Synthesis and look through the synthesis report.
Document somewhere what you see under *Performance & Resource Estimates*, so that it can be compared against versions of the kernel after we have optimized it.
What is the latency of the module(krnl_fir)? What is its initiation interval? and how many BRAMs, DSPs, FFs and LUTs does the tool estimate that it will require?

Before trying to optimize the performance, we should make sure that this implementation works. Start by running C simulation, and if that works, also run C/RTL Cosimulation. These should both print "Test PASSED!" if they worked correctly. 

Now to the optimization. When you looked at `krnl_fir.cpp` before you might have noticed that it takes one input value and generates one output value every time we execute the function. The synthesis report probably showed an interval greater or equal to the latency, meaning the kernel will not be accepting a new input value until it is fully done with its current one. For a FIR filter we should be able to do a lot better, probably accepting an input every clock cycle.

To do this, we can update the code and optimize the implemented algorithm, or we can instruct the compiler how to better synthesis this code with some hints. In Vitis HLS these hints come in the form of C++ pragmas.

So, to get an initiation interval lower than the latency, we need to pipeline the design. Add in the following line somewhere in the `krnl_fir` function, and re-run synthesis by clicking C Synthesis>Run:
```cpp
#pragma HLS pipeline
```
How does the performance and resource requirements of this version compare to the earlier one, and does the C/RTL Cosimulation still pass? Then look at the Vitis High-Level Synthesis User Guide linked at the end of this document, and read through the documentation for the pipeline pragma we just used, this is smart to do anytime you use a new pragma, or if trying to figure out if there is some pragma that might help you transform a design in the way you need.

Now we should look a bit closer at a few more parts of the Synthesis Report. Under *SW-to-HW Mapping* we should be able to read how our function arguments are mapped to hardware interfaces.
Right now both *x* and *y* should be implemented as registers in an interface called *s_axi_control*; more details on this can be found under *S_AXILITE Registers*.
This means that the kernel implements a set of memory mapped registers to communicate both its inputs and outputs, and to control when it runs. Having to do multiple reads and writes over a memory bus(in this case AXI lite) to run one iteration of a FIR filter does not seem that efficient. A real world usecase for a digital filter might be to apply it on a stream of data from some sensor, so we should look at how to make Vitis HLS implement interfaces of this sort.

To explicitly tell Vitis HLS what interface to use, we need the `interface` pragma, insert the following pragmas at the top of the `krnl_fir` function and go through the steps you did after the last modification. Synthesis, evaluation and simulation.
```cpp
#pragma HLS INTERFACE axis port=x
#pragma HLS INTERFACE axis port=y
#pragma HLS INTERFACE ap_ctrl_none port=return
```
Setting a port to `axis` tells the compiler to try to use AXI stream for that port(which is a simple streaming protocol with back-pressure). The interface `ap_ctrl_none` is a special interface type, and can only be used with the `return` port(the return value of the function), and instructs the compiler that this function should be a free-running kernel. A free-running kernel does not have an interface to start or stop its execution, it is always running. Depending on the interfaces used for the kernels data ports, this might not be possible, which is also true for any HLS pragma. The compiler will try to follow them, but if they do not make sense with the code in question or other pragmas, it will disregard them.


## 2. Matrix multiplication

In this exercise you should try and optimize an HLS design on your own. Again start by generating the project:
```bash
make krnl_matrix
```
This adds the component `krnl_matrix` to the same workspace as we used in exercise 1.

This kernel implements matrix matrix multiplication for two 256x256 matrices of ints. In this basic version, it uses one AXI master memory interface to read and write data, and will have a latency of about 40 million clock cycles. What can you do to make this faster, how does that affect the designs resource usage?

Some ideas to explore can be:
- The number of memory interfaces.
- Loop ordering.
- Local buffering.
- Tileing/blocking.

Reading through the documentation for the `m_axi` interface, as well as for the `unroll`, `performance`, and `array_partition` pragmas will be useful for some of these.

Halving the latency shouldn't be a problem, and it is possible to get down to well below 1 million cycles. 


## 3. Sobel filter
HLS supports a few different programming models, which are better described here: https://docs.amd.com/r/2023.2-English/ug1399-vitis-hls/HLS-Programmers-Guide, but one of the the easier ways to achieve a high degree of parallelism and throughput, especially when working on streaming data as you will in the project, is task-level parallelism.
This can come in the form of control-driven TLP, using the `dataflow` pragma and breaking the program up into a set of function calls, or data-driven TLP using the hls_task library with free-running task communicating over queues/streams.

In this exercise you will implement a sobel filter using data-driven TLP.
Similarly as before, you can generate the project by running:
```bash
make krnl_sobel
```
A code skeleton for this has been provided in `krnl_sobel.cpp`, that takes in a stream of pixels, converts them from RGB values to intensity gray-scale and applies the sobel operator as the consecutive convolution by two 3x3 kernels.
You need to implement the 3x3 convolution operator by constructing a sliding window shift register, and correctly keep track of frame/pixel location to support zero padding.
A lot IP cores for processing image streams follow the standard described on page 98 of UG1037, the AXI4-Stream Video Protocol: https://docs.amd.com/v/u/en-US/ug1037-vivado-axi-reference-guide
It adds two side channels to the basic AXI stream, SOF(Start Of Frame) sent over the `tuser` signal, and EOL(End Of Line) sent over the `tlast` signal.
These signals might allow you simplify some logic,and theoretically make it possible to support arbitrary frame sizes, but you will only be required to support fixed sized frames.

## 4. Run on real hardware
Now, lets take the sobel filter you developed in the last exercise, take it through implementation and run it on an FPGA.
We will connect it to the two provided components `mm2p`(meory to pixel) and `p2mm`(pixel to memory), which reads an iamge from memory pushes it over an stream and vice versa.
Instanciating these components/kernels and connecting their stream interfaces is done using the config file `fpga/sobel.ini`, have a look at this, as well as the Makefile target for building `sobel.xclbin`, then start the build by running:
> **Note**: This takes a while, so it might be smart to start a persistent shell using tmux or screen, so the build isnt killed if your ssh connection fails.
```bash
make _x.xilinx_u55c_gen3x16_xdma_3_202210_1/sobel.xclbin
```
When this finnishes we can try it out by moving to the `fpga` folder,
```bash
cd fpga
```
and run the jupyter/python based test code:
```bash
jupyter-notebook
```
If the ssh session was launched with `-X`, this will open the jupyter notebook in a server side firefox window. If not, or if you want to run it in a local browser anyways, setup a ssh tunnel fo the specific port that your jupyter server is launched with:
```bash
ssh -L <port>:localhost:<port> <user>@<server>
```
> If you run ssh through vscode, this should be automatic.

Then, in your browser, go to the url printed in the terminal, open `sobel.ipynb` and run through the cells of the notebook.

## Notes, tips & tricks
A very useful resource is the Vitis High-Level Synthesis User Guide (UG1399)
https://docs.amd.com/r/2023.2-English/ug1399-vitis-hls/HLS-Pragmas
Especially the chapters on pragmas and libraries are great to reference when developing for HLS.

Another helpful resource is the free book "Parallel Programming for FPGAs"
https://raw.githubusercontent.com/KastnerRG/pp4fpgas/gh-pages/main.pdf

We will see a lot of AMBA protocols, like AXI or AXI Stream, when working with Vitis HLS. You don't have to know how these work under the hood, but it might be good to at least have seen how the ready-valid handshakes in AXI stream work.
- AXI: https://developer.arm.com/documentation/ihi0022/latest/
- AXI stream: https://developer.arm.com/documentation/ihi0051/latest/


## Vitis python interface
Some have commented on the slugishness of accessing the vitis GUI over an SSH tunnel, to get around this, vitis can also be controlled using a python based CLI.

After having setup a workspace, and generated the initial component, by for example running:
```bash
make krnl_fir
```
The vitis interactive python shell can then be launched by running:
```bash
vitis -i
```
Then, run something like this:
```python
client = vitis.create_client()
client.set_workspace("_x.xilinx_u55c_gen3x16_xdma_3_202210_1")
fir = client.get_component("krnl_fir")
fir.execute("C_SIMULATION") #This runs the C Simulation
fir.execute("SYNTHESIS") #This runs the C Synthesis
fir.execute("CO_SIMULATION") #This runs the C/RTL Co-simulation
```
The shell can be kept open and operations rerun after changing the source code, and the reports can be found as human readable `.rpt` files in the `reports` folder, so for `krnl_fir`:s synthesis report, this would be `_x.xilinx_u55c_gen3x16_xdma_3_202210_1/krnl_fir/krnl_fir/reports/hls_compile.rpt`.
