include ../../project.mk

TARGET       = lwip_test

# source to include
SRC_DIR      = .
SRC_DIR     += $(CORES_DIR)/core_soc/src_c
SRC_DIR     += $(PANO_FW_DIR)/lib/libstd
EXTRA_SRC   += $(PANO_FW_DIR)/common/log.c

PROJECT_DIR  := $(abspath .)
INCLUDE_PATH += ../include
INCLUDE_PATH += $(PROJECT_INC)
INCLUDE_PATH += $(PANO_FW_DIR)/common


# build optons
OPT        = 2
ENABLE_LST = yes

BUILD_TARGETS += $(LWIP_LIB)

include $(PANO_FW_DIR)/lwip/include.mk
include $(MAKE_DIR)/c_prog.mk

.PHONY: distclean

distclean: clean
	make -C $(LWIP_LIB_DIR) distclean

debug:
	echo "SRC_DIR: $(SRC_DIR)"
	echo "SRC: $(SRC)"
	echo "OBJ: $(OBJ)"
	echo "EXTRA_LIBS: $(EXTRA_LIBS)"
	echo "CFLAGS: $(CFLAGS)"
	echo "INCLUDE_PATH: $(INCLUDE_PATH)"
