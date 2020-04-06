## Panologic based USB 2.0 Sniffer with Ethernet interface 

This is a port of http://github.com/ultraembedded/usb_sniffer to the second 
generation Panologic thin client. 

## Status
This is a work in progress.  Currently this repository is being used to 
develop Ethernet hardware support for the Panologic G2 along with TCP/IP
firmware support.

The project currently builds and is able to send and receive Ethernet test 
frames via 100BaseT and 1000BaseT connections.  No USB code is present yet.

For a fully functional Panologic based USB sniffer heat up your soldering iron
and see https://github.com/skiphansen/usb_sniffer/blob/master/fpga/panologic_g2/README.md.

## HW Requirements

* A Pano Logic G2 (the one with a DVI port)
* A suitable 5 volt power supply
* A JTAG programmer to load the bitstream into the FPGA.

## Software Requirements

* GNU make
* xc3sprog

To program the SPI flash in the Pano and/or to run this project you DO NOT 
need Xilinx's ISE.

If you would like to modify the firmware you'll also need gcc built for 
RISC-V RV32IM.

If you would like to modify the RTL you'll also need Xilinx ISE 14.7.

The free Webpack version of Xilinx [ISE 14.7](https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vivado-design-tools/archive-ise.html) is used for development.
Download the latest Windows 10 version which supports the Spartan 6 family of 
chips used in the second generation Panologic device.

## Serial port 

A serial port is required for debugging.

Please see the [fpga_test_soc](https://github.com/skiphansen/fpga_test_soc/tree/master/fpga/panologic_g2#serial-port) for connection information.

If the serial port you use to interface to the Pano is not /dev/ttyUSB1 then
you will need to set the "TARGET_PORT" environment variable to point
to your serial device.

Ultraembedded's SOC platform includes the ability to load firmware over a 
serial port which is VERY HANDY for code development.

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

Once xc3sprog has been in installed the bit file can be programmed into the 
Pano's SPI flash by running "make prog_fpga".

### Running

1. If necessary set the "PLATFORM", "CABLE" and "TARGET_PORT" environment 
   variables as needed for your environment (see above).

2. Run "make load" to load the bitstream into the Pano.

3. Run "make start_console" to load the test app and connect to the console.

```
skip@dell-790:~/pano/working/panog2_usb_sniffer$ make load
make -C fw/eth_test load
make[1]: Entering directory '/home/skip/pano/working/panog2_usb_sniffer/fw/eth_test'
make[2]: Entering directory '/home/skip/pano/working/panog2_usb_sniffer/fpga'
xc3sprog -c jtaghs2 -v /home/skip/pano/working/panog2_usb_sniffer/prebuilt/pano-g2.bit
XC3SPROG (c) 2004-2011 xc3sprog project $Rev: 774 $ OS: Linux
Free software: If you contribute nothing, expect nothing!
Feedback on success/failure/enhancement requests:
        http://sourceforge.net/mail/?group_id=170565
Check Sourceforge for updates:
        http://sourceforge.net/projects/xc3sprog/develop

Using built-in device list
Using built-in cable list
Cable jtaghs2 type ftdi VID 0x0403 PID 0x6014 Desc "Digilent USB Device" dbus data e8 enable eb cbus data 00 data 60
Using Libftdi, Using JTAG frequency   6.000 MHz from undivided clock
JTAG chainpos: 0 Device IDCODE = 0x3401d093     Desc: XC6SLX150
Created from NCD file: fpga_routed.ncd;HW_TIMEOUT=FALSE;UserID=0xFFFFFFFF
Target device: 6slx150fgg484
Created: 2020/04/06 14:57:36
Bitstream length: 16779664 bits
DNA is 0x1927bec012fa2fff
done. Programming time 2878.8 ms
USB transactions: Write 1041 read 14 retries 9
make[2]: Leaving directory '/home/skip/pano/working/panog2_usb_sniffer/fpga'
make[1]: Leaving directory '/home/skip/pano/working/panog2_usb_sniffer/fw/eth_test'
skip@dell-790:~/pano/working/panog2_usb_sniffer$ make start_console
make -C fw/eth_test start_console
make[1]: Entering directory '/home/skip/pano/working/panog2_usb_sniffer/fw/eth_test'
/home/skip/pano/working/panog2_usb_sniffer/pano/tools/dbg_bridge/console.py -d /dev/ttyUSB.Pano -b 1000000
/home/skip/pano/working/panog2_usb_sniffer/pano/tools/dbg_bridge/poke.py -t uart -d /dev/ttyUSB.Pano -b 1000000 -a 0xF0000000 -v 0x0
Traceback (most recent call last):
  File "/home/skip/pano/working/panog2_usb_sniffer/pano/tools/dbg_bridge/poke.py", line 28, in <module>
    main(sys.argv[1:])
  File "/home/skip/pano/working/panog2_usb_sniffer/pano/tools/dbg_bridge/poke.py", line 25, in main
    bus_if.write32(addr, value)
  File "/home/skip/pano/working/panog2_usb_sniffer/pano/tools/dbg_bridge/bus_interface.py", line 61, in write32
    return self.bus.write32(addr, value)
  File "/home/skip/pano/working/panog2_usb_sniffer/pano/tools/dbg_bridge/uart_bus_interface.py", line 78, in write32
    self.connect()
  File "/home/skip/pano/working/panog2_usb_sniffer/pano/tools/dbg_bridge/uart_bus_interface.py", line 44, in connect
    raise Exception("Target not responding correctly, check interface / baud rate...")
Exception: Target not responding correctly, check interface / baud rate...
/home/skip/pano/working/panog2_usb_sniffer/pano/tools/dbg_bridge/console-uart.py -t uart -d /dev/ttyUSB.Pano -b 1000000
Ethernet Status: 0x44
Link is down
Link speed: 100m
Ethernet Status: 0xa0
Link is up
Link speed: 1g
1: Read 60 (0x3c) bytes from Rx Fifo:
0xff 0xff 0xff 0xff 0xff 0xff 0x00 0x00 0x00 0x00 0x00 0x00 0x88 0x99 0x03 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
2: Read 60 (0x3c) bytes from Rx Fifo:
0xff 0xff 0xff 0xff 0xff 0xff 0x00 0x00 0x00 0x00 0x00 0x00 0x88 0x99 0x03 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
Another RX frame is waiting
3: Read 60 (0x3c) bytes from Rx Fifo:
0xff 0xff 0xff 0xff 0xff 0xff 0x00 0x04 0xf2 0x33 0x71 0x3f 0x08 0x06 0x00 0x01
0x08 0x00 0x06 0x04 0x00 0x01 0x00 0x04 0xf2 0x33 0x71 0x3f 0xc0 0xa8 0x7b 0xd7
0x00 0x00 0x00 0x00 0x00 0x00 0xc0 0xa8 0x7b 0x77 0x95 0xf0 0x2a 0xe0 0x95 0xf4
0xdc 0x40 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
```

### Building everything from sources

**NB:** While it may be possible to use Windows for development I haven't 
tried it and don't recommend it.

1. Clone the https://github.com/skiphansen/panog2_usb_sniffer repository
2. cd into .../panog2_usb_sniffer
3. Make sure the RISC-V GCC (built for RV32IM) is in the PATH.
4. Run "make build_all" or "make PLATFORM=pano-g2-c build_all".


## Acknowledgement and Thanks
This project uses code from several other projects including:
 - [https://github.com/ultraembedded/fpga_test_soc.git]
 - [https://github.com/yol/ethernet_mac.git]

## Pano Links

Link to other Panologic information can be found [here](https://github.com/skiphansen/pano_blocks#pano-links)

## LEGAL 

My original work (the Pano Codec glue code) is released under the GNU General 
Public License, version 2.

