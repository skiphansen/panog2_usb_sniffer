###############################################################################
## Makefile
###############################################################################
include ../project.mk
CPU        ?= riscv

# Choice: [rv32i, rv32i_spartan6, rv32im, rv32imsu]
RISCV_CORE  ?= rv32i_spartan6

SRC_DIR       = .
SRC_DIR      += $(CORES_DIR)/core_soc/src_v
SRC_DIR      += $(CORES_DIR)/dbg_bridge/src_v
SRC_DIR      += $(CORES_DIR)/cpu/riscv/core/$(RISCV_CORE)
SRC_DIR      += $(CORES_DIR)/cpu/riscv/top_tcm_wrapper
SRC_DIR      += $(CORES_DIR)/ethernet_mac
SRC_DIR      += $(CORES_DIR)/ethernet_mac/xilinx
SRC_DIR      += $(CORES_DIR)/ethernet_mac/xilinx/ipcore_dir
#SRC_DIR	     += ${CORES_DIR}/cores/ulpi_wrapper/rtl
#SRC_DIR	     += ${CORES_DIR}/cores/usb_sniffer/rtl

EXTRA_VFLAGS += CPU_SELECT_RISCV=1

# EXCLUDE_SRC - list of core files with local replacements
EXCLUDE_SRC += $(CORES_DIR)/core_soc/src_v/axi4lite_dist.v
EXCLUDE_SRC += $(CORES_DIR)/core_soc/src_v/core_soc.v
EXCLUDE_SRC += $(CORES_DIR)/core_soc/src_v/spi_lite.v
EXCLUDE_SRC += $(CORES_DIR)/ethernet_mac/ethernet_mac_tb.vhd

BSCAN_SPI_DIR = $(PANO_CORES_DIR)/xc3sprog

COMPRESS_BITFILE = yes
INIT_IMAGE = ./firmware.mem

#MAP_CMDS = -w -intstyle ise -detail -ir off -ignore_keep_hierarchy -ol high

# NB: the -pr b option was removed from the default options, otherwise there 
# are timing errors
MAP_CMDS = -w -intstyle ise -detail -ir off -ignore_keep_hierarchy -timing -ol high -mt 2

# use 4 cores
PAR_CMDS = -w -intstyle ise -ol high  -mt 4

PATCHED_ETH_MAC = $(CORES_DIR)/ethernet_mac/.patched
PATCHED_CPU = $(CORES_DIR)/cpu/riscv/.patched

.PHONY: init_and_build
init_and_build: $(PATCHED_ETH_MAC) $(PATCHED_CPU) all

TOPDIR = ..
include $(TOPDIR)/pano/make/ise.mk

ETHERNET_MAC_PATCH := $(PATCHES_DIR)/cores/ethernet_mac/0001-BUFGMUX-routing-fix.patch

$(PATCHED_ETH_MAC): $(ETHERNET_MAC_PATCH)
	(cd $(CORES_DIR)/ethernet_mac; git reset HEAD --hard)
	(cd $(CORES_DIR)/ethernet_mac; patch -p1 < $(ETHERNET_MAC_PATCH))
	touch $@

CPU_PATCH := $(PATCHES_DIR)/cores/cpu/riscv/0001-Increase-on-chip-RAM-to-128k.patch

$(PATCHED_CPU): $(CPU_PATCH)
	(cd $(CORES_DIR)/cpu/riscv; git reset HEAD --hard)
	(cd $(CORES_DIR)/cpu/riscv; patch -p1 < $(CPU_PATCH))
	touch $@

firmware.mem:
	make -C $(TOPDIR)/fw/lwip_test init_image

