include ../../project.mk

TARGET       = lwip_test

# source to include
SRC_DIR      = .
SRC_DIR     += $(CORES_DIR)/core_soc/src_c
SRC_DIR     += $(PANO_FW_DIR)/lib/libstd
EXTRA_SRC   += $(PANO_FW_DIR)/common/log.c
SRC_DIR	    += ../lwip/src/core
SRC_DIR	    += ../lwip/src/core/ipv4
EXTRA_SRC   += ../lwip/src/netif/ethernet.c

INCLUDE_PATH += ../include
INCLUDE_PATH += $(PANO_FW_DIR)/common
INCLUDE_PATH += ../lwip/src/include

# build optons
OPT        = 2
ENABLE_LST = yes

include $(TOPDIR)/pano/make/c_prog.mk

