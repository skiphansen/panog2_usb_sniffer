## Panologic based USB 2.0 Sniffer with Ethernet interface 

This is a port of http://github.com/ultraembedded/usb_sniffer to the second 
generation Panologic thin client. 

## Status
This is a work in progress.  Currently this repository is being used to 
develop Ethernet hardware support for the Panologic G2 along with TCP/IP
firmware support.

The project currently builds and is able to send and recieve Ethernet test 
frames via 100BaseT and 1000BaseT connections.  No USB code is present yet.

For a fully functional Panologic based USB sniffer heat up your soldering iron
and see https://github.com/skiphansen/usb_sniffer/blob/master/fpga/panologic_g2/README.md.

## HW Requirements

* A Pano Logic G2 (the one with a DVI port)
* A suitable 5 volt power supply
* A JTAG programmer to load the bitstream into the FPGA.

## Software Requirements

* GNU make
* Xilinx ISE 14.7
* xc3sprog

The free Webpack version of Xilinx [ISE 14.7](https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vivado-design-tools/archive-ise.html) is used for development.
Download the latest Windows 10 version which supports the Spartan 6 family of 
chips used in the second generation Panologic device.

## Serial port 

A serial port is required for debugging.

Please see the [fpga_test_soc](https://github.com/skiphansen/fpga_test_soc/tree/master/fpga/panologic_g2#serial-port) for connectin information.

If the serial port you use to interface to the Pano is not /dev/ttyUSB1 then
then you will need to set the "TARGET_PORT" environment variable to point
to your serial device.

Ultraembedded's SOC platform includes the ability to load firmware over a 
serial port which is VERY HANDY for code development.

### Building everything from sources

**NB:** While it may be possible to use Windows for development I haven't 
tried it and don't recommend it.

1. Clone the https://github.com/skiphansen/panog2_usb_sniffer repository
2. cd into .../panog2_usb_sniffer
3. Make sure the RISC-V GCC (built for RV32IM) is in the PATH.
4. Run "make build_all" or "make PLATFORM=pano-g2-c build_all".

## Programming the Pano

Install xc3sprog for your system.  If a binary install isn't available for your
system the original project can be found here: https://sourceforge.net/projects/xc3sprog/.
Sources can be checked out using subversion from https://svn.code.sf.net/p/xc3sprog/code/trunk.

As an alternate if you don't have subversion a fork of the original project
can be found here: https://github.com/Ole2mail/xc3sprog.git .

If your JTAG cable is not a Digilent JTAG-HS2 then you will need to set the
"CABLE" environment variable to your cable type before loading the bit file.

Refer to the supported hardware [web page](http://xc3sprog.sourceforge.net/hardware.php) page or run  xc3sprog -c 
to find the correct cable option for your device.

**IMPORTANT: There are 2 versions of the Pano Logic G2: some use a Spartan 6 
LX150 while others use an LX100. You should be able to distinguish between the 
two by the revision code: LX150 is rev B and LX100 is rev C.  

The bit file and the embedded firmware must be generated for the correct device, 
the binary images are NOT compatible.  The build system uses the PLATFORM 
environment variable to determine the target device.  If the PLATFORM environment 
variable is not set a revision A or B device is assumed.

Set the PLATFORM environment variable to "pano-g2-c" if your device is a 
revision C before running make or specify it on the make command line.

Once xc3sprog has been in installed the bit file and SPI filesystem can be
programmed into the Pano's SPI flash by running "make prog_all".

### Running

1. If necessary set the "CABLE" and "TARGET_PORT" environment variables
   as needed for your environment (see above)
2. Build everything (see above)
3. cd into .../panog2_usb_sniffer/fw/eth_test
4. Run "make load" to load the fpga bit file
5. Run "make run" to load the test app and connect to the console.


## Acknowledgement and Thanks
This project uses code from several other projects including:
 - [https://github.com/ultraembedded/fpga_test_soc.git]
 - [https://github.com/yol/ethernet_mac.git]

## Pano Links

Link to other Panologic information can be found [here](https://github.com/skiphansen/pano_blocks#pano-links)

## LEGAL 

My original work (the Pano Codec glue code) is released under the GNU General 
Public License, version 2.

