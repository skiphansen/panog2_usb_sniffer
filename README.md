## Panologic based USB 2.0 Sniffer with Ethernet interface 

This is a port of http://github.com/ultraembedded/usb_sniffer to the second 
generation Panologic thin client. 

## Status
This is a work in progress.  Currently this repository is being used to 
develop Ethernet hardware support for the Panologic G2 along with TCP/IP
firmware support.

No USB code is present yet. For a fully functional Panologic based USB sniffer 
heat up your soldering iron and see https://github.com/skiphansen/usb_sniffer/blob/master/fpga/panologic_g2/README.md.

This project currently builds and is able to obtain an IP address from an
DHCP server via 10BaseT, 100BaseT or 1000BaseT full duplex connections.  

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

For some reason that I don't understand both eth_test nor the lwip_test hang
if loaded immediately after loading the bit file, but they run fine if the
blinky application is run first.

To work around this problem load the bit file, run blinky, hit Ctrl-C and
then run lwip_test.  Pull requests to fix this bug would be MOST welcome!

```
skip@dell-790:~/pano/localref/panog2_usb_sniffer/fw/lwip_test$ (cd ../blinky/;make load;make run)
make[1]: Entering directory '/home/skip/pano/localref/panog2_usb_sniffer/fpga'
xc3sprog -c jtaghs2 -v /home/skip/pano/localref/panog2_usb_sniffer/prebuilt/pano-g2.bit
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
Created: 2020/04/08 18:49:21
Bitstream length: 17921600 bits
DNA is 0x1927bec012fa2fff
done. Programming time 3075.8 ms
USB transactions: Write 1111 read 14 retries 10
make[1]: Leaving directory '/home/skip/pano/localref/panog2_usb_sniffer/fpga'
/home/skip/pano/localref/panog2_usb_sniffer/pano/tools/dbg_bridge/run.py -d /dev/ttyUSB.Pano -b 1000000 -f  build/blinky
/home/skip/pano/localref/panog2_usb_sniffer/pano/tools/dbg_bridge/poke.py -t uart -d /dev/ttyUSB.Pano -b 1000000 -a 0xF0000000 -v 0x0
/home/skip/pano/localref/panog2_usb_sniffer/pano/tools/dbg_bridge/load.py -t uart -d /dev/ttyUSB.Pano -b 1000000 -f build/blinky -p ''
ELF: Loading 0x0 - size 9KB
 |XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX| 100.0%
/home/skip/pano/localref/panog2_usb_sniffer/pano/tools/dbg_bridge/console-uart.py -t uart -d /dev/ttyUSB.Pano -b 1000000
Traceback (most recent call last):
  File "/home/skip/pano/localref/panog2_usb_sniffer/pano/tools/dbg_bridge/console-uart.py", line 50, in <module>
    main(sys.argv[1:])
  File "/home/skip/pano/localref/panog2_usb_sniffer/pano/tools/dbg_bridge/console-uart.py", line 44, in main
    ch = stdio_read()
  File "/home/skip/pano/localref/panog2_usb_sniffer/pano/tools/dbg_bridge/stdio.py", line 32, in stdio_read
    ch = os.read(sys.stdin.fileno(), 1)
KeyboardInterrupt

skip@dell-790:~/pano/localref/panog2_usb_sniffer/fw/lwip_test$ make run
/home/skip/pano/localref/panog2_usb_sniffer/pano/tools/dbg_bridge/run.py -d /dev/ttyUSB.Pano -b 1000000 -f  build/lwip_test
/home/skip/pano/localref/panog2_usb_sniffer/pano/tools/dbg_bridge/poke.py -t uart -d /dev/ttyUSB.Pano -b 1000000 -a 0xF0000000 -v 0x0
/home/skip/pano/localref/panog2_usb_sniffer/pano/tools/dbg_bridge/load.py -t uart -d /dev/ttyUSB.Pano -b 1000000 -f build/lwip_test -p ''
ELF: Loading 0x0 - size 60KB
 |XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX| 100.0%
/home/skip/pano/localref/panog2_usb_sniffer/pano/tools/dbg_bridge/console-uart.py -t uart -d /dev/ttyUSB.Pano -b 1000000
Hello pano world!
pano_netif_init: pano_netif_init: called
dhcp_start(netif=ef98) e0u
dhcp_start(): mallocing new DHCP client
dhcp_start(): allocated dhcpdhcp_start(): starting DHCP configuration
init_default_netif: gNetif.output 0xa454
ClearRxFifo: Clearing Rx FIFO
ClearRxFifo: FIFO cleared after 186 reads
Ethernet Status: 0xa4
Link is up
Link speed: 1g
dhcp_discover()
transaction id xid(4080601)
dhcp_discover: making request
dhcp_discover: sendto(DISCOVER, IP_ADDR_BROADCAST, LWIP_IANA_PORT_DHCP_SERVER)
pano_netif_output: called tot_len: 350, len: 350:
ff ff ff ff ff ff 00 1c 02 70 1d 5d 08 00 45 00
01 50 00 00 00 00 ff 11 ba 9d 00 00 00 00 ff ff
ff ff 00 44 00 43 01 3c fb 81 01 01 06 00 04 08
06 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 1c 02 70 1d 5d 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 63 82 53 63 35 01 01 39 02 05
dc 37 03 01 03 1c ff 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00
dhcp_discover: deleting()
dhcp_discover: SELECTING
dhcp_discover(): set request timeout u msecs
pano_netif_poll: Rx #1: Read 342 (0x156) bytes from Rx Fifo:
0x00 0x1c 0x02 0x70 0x1d 0x5d 0x4c 0xe6 0x76 0x3e 0xf3 0x1a 0x08 0x00 0x45 0x00
0x01 0x48 0xf6 0x68 0x00 0x00 0x40 0x11 0x0b 0x79 0xc0 0xa8 0x7b 0x01 0xc0 0xa8
0x7b 0x71 0x00 0x43 0x00 0x44 0x01 0x34 0x08 0x5c 0x02 0x01 0x06 0x00 0x04 0x08
0x06 0x01 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xc0 0xa8 0x7b 0x71 0xc0 0xa8
0x7b 0x01 0x00 0x00 0x00 0x00 0x00 0x1c 0x02 0x70 0x1d 0x5d 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x63 0x82 0x53 0x63 0x35 0x01 0x02 0x36 0x04 0xc0
0xa8 0x7b 0x01 0x33 0x04 0x00 0x00 0xa8 0xc0 0x3a 0x04 0x00 0x00 0x54 0x60 0x3b
0x04 0x00 0x00 0x93 0xa8 0x01 0x04 0xff 0xff 0xff 0x00 0x1c 0x04 0xc0 0xa8 0x7b
0xff 0x03 0x04 0xc0 0xa8 0x7b 0x01 0xff 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00
dhcp_recv(pbuf = 11960) from DHCP server u.u.u.u port u
pbuf->len = u
pbuf->tot_len = u
skipping option u in options
searching DHCP_OPTION_MESSAGE_TYPE
DHCP_OFFER received in DHCP_STATE_SELECTING state
dhcp_handle_offer(netif=ef98) e0u
dhcp_handle_offer(): server 0x017ba8c0
dhcp_handle_offer(): offer for 0x717ba8c0
dhcp_select(netif=ef98) e0u
transaction id xid(4080601)
pano_netif_output: called tot_len: 350, len: 350:
ff ff ff ff ff ff 00 1c 02 70 1d 5d 08 00 45 00
01 50 00 01 00 00 ff 11 ba 9c 00 00 00 00 ff ff
ff ff 00 44 00 43 01 3c 2c a2 01 01 06 00 04 08
06 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 1c 02 70 1d 5d 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 63 82 53 63 35 01 03 39 02 05
dc 32 04 c0 a8 7b 71 36 04 c0 a8 7b 01 37 03 01
03 1c ff 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00
dhcp_select: REQUESTING
dhcp_select(): set request timeout u msecs
pano_netif_poll: Rx #2: Read 342 (0x156) bytes from Rx Fifo:
0x00 0x1c 0x02 0x70 0x1d 0x5d 0x4c 0xe6 0x76 0x3e 0xf3 0x1a 0x08 0x00 0x45 0x00
0x01 0x48 0xf6 0x69 0x00 0x00 0x40 0x11 0x0b 0x78 0xc0 0xa8 0x7b 0x01 0xc0 0xa8
0x7b 0x71 0x00 0x43 0x00 0x44 0x01 0x34 0x05 0x5c 0x02 0x01 0x06 0x00 0x04 0x08
0x06 0x01 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xc0 0xa8 0x7b 0x71 0xc0 0xa8
0x7b 0x01 0x00 0x00 0x00 0x00 0x00 0x1c 0x02 0x70 0x1d 0x5d 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00 0x63 0x82 0x53 0x63 0x35 0x01 0x05 0x36 0x04 0xc0
0xa8 0x7b 0x01 0x33 0x04 0x00 0x00 0xa8 0xc0 0x3a 0x04 0x00 0x00 0x54 0x60 0x3b
0x04 0x00 0x00 0x93 0xa8 0x01 0x04 0xff 0xff 0xff 0x00 0x1c 0x04 0xc0 0xa8 0x7b
0xff 0x03 0x04 0xc0 0xa8 0x7b 0x01 0xff 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
0x00 0x00 0x00 0x00 0x00 0x00
dhcp_recv(pbuf = 11960) from DHCP server u.u.u.u port u
pbuf->len = u
pbuf->tot_len = u
skipping option u in options
searching DHCP_OPTION_MESSAGE_TYPE
DHCP_ACK received
dhcp_bind(netif=ef98) e0u
dhcp_bind(): t0 renewal timer  secs
dhcp_bind(): set request timeout  msecs
dhcp_bind(): t1 renewal timer  secs
dhcp_bind(): set request timeout  msecs
dhcp_bind(): t2 rebind timer  secs
dhcp_bind(): set request timeout  msecs
dhcp_bind(): IP: 0x717ba8c0 SN: 0x00ffffff GW: 0x017ba8c0
```

## Ethernet Status lights

Note:  The RTL handles Ethernet the link configuration negotiation and it only
allow full duplex connections at 1000BaseT, 100BaseT and 10BaseT.  MDI/MDIX
configuration is automatic.  Links WILL NOT be established with half duplex
hubs/switches (this really shouldn't be an issue).

Looking at the Pano's Ethernet port with the tab up the right LED shows the
link state

Off - No Link
Green - 10BaseT connection
Green/Amber - 100BaseT connection
Amber - 1000BaseT connection


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

My original work (the Pano ethernet_mac glue code) is released under the 
GNU General Public License, version 2.

