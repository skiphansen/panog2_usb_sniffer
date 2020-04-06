.PHONY: help prog_all build_all prog_fpga clean_all

INIT_APP := fw/eth_test

help:
	@echo "Usage:"
	@echo "   REV A or B Pano (xc6slx150):"
	@echo "      make prog_all  - program SPI flash"
	@echo "      make build_all - rebuild all images from sources (optional)"
	@echo
	@echo "   REV C Pano (xc6slx100):"
	@echo "      make PLATFORM=pano-g2-c prog_all"
	@echo "      make PLATFORM=pano-g2-c build_all"
	@echo "  other make targets: prog_fpga, clean_all"
     
prog_all:
	make -C fpga prog_fpga

build_all:
	make -C $(INIT_APP) init_image
	make -C fpga

prog_fpga:
	make -C fpga prog_fpga

clean_all:
	make -C $(INIT_APP) clean
	make -C fpga clean

