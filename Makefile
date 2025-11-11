.PHONY: help

help:
	@echo "Makefile Usage:"
	@echo "  make all DEVICE=<FPGA platform> kernel_list=\"<list of kernels>"\"
	@echo "      Command to generate the xo for specified device and Interface."
	@echo "      By default, DEVICE=xilinx_u280_xdma_201920_3 and kernel_list=\"krnl_s2mm krnl_mm2s\""
	@echo ""
	@echo "  make clean "
	@echo "      Command to remove the generated non-hardware files."
	@echo ""
	@echo "  make distclean"
	@echo "      Command to remove all the generated files."
	@echo ""


DEVICE ?= xilinx_u55c_gen3x16_xdma_3_202210_1
kernel_list ?= krnl_fir krnl_matrix krnl_sobel

FREQ ?= 300000000
XSA := $(strip $(patsubst %.xpfm, % , $(shell basename $(DEVICE))))
TEMP_DIR := _x.$(XSA)
VPP := $(XILINX_VITIS)/bin/v++
BINARY_OBJS = $(addprefix $(TEMP_DIR)/, $(addsuffix .xo, $(kernel_list)))


.PHONY: all clean distclean
all: check-devices check-vitis check-xrt $(BINARY_OBJS)


# Cleaning stuff
clean:
	rm -rf *v++* *.log *.jou

distclean: clean
	rm -rf _x* .Xil

krnl_matrix: $(TEMP_DIR)/krnl_matrix.xo
krnl_fir: $(TEMP_DIR)/krnl_fir.xo
krnl_sobel: $(TEMP_DIR)/krnl_sobel.xo

$(TEMP_DIR)/krnl_matrix.xo: src/krnl_matrix.cpp src/krnl_matrix.h src/tb_matrix.cpp krnl_matrix.cfg
	$(eval kernel := $(subst $(TEMP_DIR)/,,$(subst .xo,,$@)))
	mkdir -p $(TEMP_DIR)/$(kernel)/$(kernel)
	$(VPP) --compile --mode hls --work_dir $(TEMP_DIR)/$(kernel)/$(kernel) --config krnl_matrix.cfg src/krnl_matrix.cpp
	ln -srnf $(TEMP_DIR)/$(kernel)/$(kernel)/$(kernel).xo $@

$(TEMP_DIR)/krnl_fir.xo: src/krnl_fir.cpp src/krnl_fir.h src/tb_fir.cpp krnl_fir.cfg
	$(eval kernel := $(subst $(TEMP_DIR)/,,$(subst .xo,,$@)))
	mkdir -p $(TEMP_DIR)/$(kernel)/$(kernel)
	$(VPP) --compile --mode hls --work_dir $(TEMP_DIR)/$(kernel)/$(kernel) --config krnl_fir.cfg src/krnl_fir.cpp
	ln -srnf $(TEMP_DIR)/$(kernel)/$(kernel)/$(kernel).xo $@

$(TEMP_DIR)/krnl_sobel.xo: src/krnl_sobel.cpp src/krnl_sobel.h src/tb_sobel.cpp krnl_sobel.cfg rtl/*
	$(eval kernel := $(subst $(TEMP_DIR)/,,$(subst .xo,,$@)))
	mkdir -p $(TEMP_DIR)/$(kernel)/$(kernel)
	$(VPP) --compile --mode hls --work_dir $(TEMP_DIR)/$(kernel)/$(kernel) --config krnl_sobel.cfg src/krnl_sobel.cpp
	ln -srnf $(TEMP_DIR)/$(kernel)/$(kernel)/$(kernel).xo $@

# FPGA Implementation of Sobel Kernel with AXI4-Stream Interfaces

$(TEMP_DIR)/mm2p.xo: src/mm2p.cpp src/krnl_sobel.h fpga/mm2p.cfg
	$(eval kernel := mm2p)
	mkdir -p $(TEMP_DIR)/$(kernel)/$(kernel)
	$(VPP) --compile --mode hls --work_dir $(TEMP_DIR)/$(kernel)/$(kernel) --config fpga/mm2p.cfg src/mm2p.cpp
	ln -srnf $(TEMP_DIR)/$(kernel)/$(kernel)/$(kernel).xo $@

$(TEMP_DIR)/p2mm.xo: src/p2mm.cpp src/krnl_sobel.h fpga/p2mm.cfg
	$(eval kernel := p2mm)
	mkdir -p $(TEMP_DIR)/$(kernel)/$(kernel)
	$(VPP) --compile --mode hls --work_dir $(TEMP_DIR)/$(kernel)/$(kernel) --config fpga/p2mm.cfg src/p2mm.cpp
	ln -srnf $(TEMP_DIR)/$(kernel)/$(kernel)/$(kernel).xo $@

$(TEMP_DIR)/sobel.xclbin: $(TEMP_DIR)/p2mm.xo $(TEMP_DIR)/mm2p.xo $(TEMP_DIR)/krnl_sobel.xo fpga/sobel.ini
	$(VPP) -t hw --platform $(DEVICE) --config fpga/sobel.ini -l -o $@ $(TEMP_DIR)/p2mm.xo $(TEMP_DIR)/mm2p.xo $(TEMP_DIR)/krnl_sobel.xo -j 4

check-devices:
ifndef DEVICE
	$(error DEVICE not set. Please set the DEVICE properly and rerun. Run "make help" for more details.)
endif

#Checks for XILINX_VITIS
check-vitis:
ifndef XILINX_VITIS
	$(error XILINX_VITIS variable is not set, please set correctly and rerun)
endif

#Checks for XILINX_XRT
check-xrt:
ifndef XILINX_XRT
	$(error XILINX_XRT variable is not set, please set correctly and rerun)
endif