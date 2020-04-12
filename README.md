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

For some reason that I don't understand both eth_test nor the lwip_test hang
if loaded immediately after loading the bit file, but they run fine if the
blinky application is run first.

To work around this problem load the bit file, run blinky, hit Ctrl-C and
then run lwip_test (pull requests to fix this bug would be MOST welcome!).

lwip_test should obtain an IP address from an DHCP on your local LAN and then
wait for Telnet connections.  It will also respond to pings.

```
skip@dell-790:~/pano/working/panog2_usb_sniffer/fw/lwip_test$ (cd ../blinky/;make load;make run)
make[1]: Entering directory '/home/skip/pano/working/panog2_usb_sniffer/fpga'
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
Created: 2020/04/10 14:01:34
Bitstream length: 17921600 bits
DNA is 0x1927bec012fa2fff
done. Programming time 3075.1 ms
USB transactions: Write 1111 read 14 retries 10
make[1]: Leaving directory '/home/skip/pano/working/panog2_usb_sniffer/fpga'
/home/skip/pano/working/panog2_usb_sniffer/pano/tools/dbg_bridge/run.py -d /dev/ttyUSB.Pano -b 1000000 -f  build/blinky
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
/home/skip/pano/working/panog2_usb_sniffer/pano/tools/dbg_bridge/load.py -t uart -d /dev/ttyUSB.Pano -b 1000000 -f build/blinky -p ''
ELF: Loading 0x0 - size 9KB
 |XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX| 100.0%
/home/skip/pano/working/panog2_usb_sniffer/pano/tools/dbg_bridge/console-uart.py -t uart -d /dev/ttyUSB.Pano -b 1000000
Traceback (most recent call last):
  File "/home/skip/pano/working/panog2_usb_sniffer/pano/tools/dbg_bridge/console-uart.py", line 50, in <module>
    main(sys.argv[1:])
  File "/home/skip/pano/working/panog2_usb_sniffer/pano/tools/dbg_bridge/console-uart.py", line 44, in main
    ch = stdio_read()
  File "/home/skip/pano/working/panog2_usb_sniffer/pano/tools/dbg_bridge/stdio.py", line 32, in stdio_read
    ch = os.read(sys.stdin.fileno(), 1)
KeyboardInterrupt

skip@dell-790:~/pano/localref/panog2_usb_sniffer/fw/lwip_test$ make run
/home/skip/pano/localref/panog2_usb_sniffer/pano/tools/dbg_bridge/run.py -d /dev/ttyUSB.Pano -b 1000000 -f  build/lwip_test
/home/skip/pano/localref/panog2_usb_sniffer/pano/tools/dbg_bridge/poke.py -t uart -d /dev/ttyUSB.Pano -b 1000000 -a 0xF0000000 -v 0x0
/home/skip/pano/localref/panog2_usb_sniffer/pano/tools/dbg_bridge/load.py -t uart -d /dev/ttyUSB.Pano -b 1000000 -f build/lwip_test -p ''
ELF: Loading 0x0 - size 98KB
 |XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX| 100.0%
/home/skip/pano/localref/panog2_usb_sniffer/pano/tools/dbg_bridge/console-uart.py -t uart -d /dev/ttyUSB.Pano -b 1000000
Hello pano world!
pano_netif_init: pano_netif_init: called
dhcp_start(netif=187c4) et0
dhcp_start(): mallocing new DHCP client
dhcp_start(): allocated dhcpdhcp_start(): starting DHCP configuration
ClearRxFifo: Clearing Rx FIFO
ClearRxFifo: FIFO cleared after 62 reads
Ethernet Status: 0xa4
Link is up
Link speed: 1g
dhcp_discover()
transaction id xid(9dcca8c5)
dhcp_discover: making request
dhcp_discover: sendto(DISCOVER, IP_ADDR_BROADCAST, LWIP_IANA_PORT_DHCP_SERVER)
pano_netif_output: called tot_len: 350, len: 350:
ff ff ff ff ff ff 00 1c 02 70 1d 5d 08 00 45 00
01 50 00 00 00 00 ff 11 ba 9d 00 00 00 00 ff ff
ff ff 00 44 00 43 01 3c be f8 01 01 06 00 9d cc
a8 c5 00 00 00 00 00 00 00 00 00 00 00 00 00 00
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
dhcp_discover(): set request timeout 2000 msecs
Rx #0: Read 342 (0x156) bytes from Rx Fifo:
00 1c 02 70 1d 5d 4c e6 76 3e f3 1a 08 00 45 00
01 48 89 01 00 00 40 11 78 e0 c0 a8 7b 01 c0 a8
7b 71 00 43 00 44 01 34 cb d2 02 01 06 00 9d cc
a8 c5 00 00 00 00 00 00 00 00 c0 a8 7b 71 c0 a8
7b 01 00 00 00 00 00 1c 02 70 1d 5d 00 00 00 00
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
00 00 00 00 00 00 63 82 53 63 35 01 02 36 04 c0
a8 7b 01 33 04 00 00 a8 c0 3a 04 00 00 54 60 3b
04 00 00 93 a8 01 04 ff ff ff 00 1c 04 c0 a8 7b
ff 03 04 c0 a8 7b 01 ff 00 00 00 00 00 00 00 00
00 00 00 00 00 00
dhcp_recv(pbuf = 1b2dc) from DHCP server 192.168.123.1 port 67
pbuf->len = 300
pbuf->tot_len = 300
skipping option 28 in options
searching DHCP_OPTION_MESSAGE_TYPE
DHCP_OFFER received in DHCP_STATE_SELECTING state
dhcp_handle_offer(netif=187c4) et0
dhcp_handle_offer(): server 0x017ba8c0
dhcp_handle_offer(): offer for 0x717ba8c0
dhcp_select(netif=187c4) et0
transaction id xid(9dcca8c5)
pano_netif_output: called tot_len: 350, len: 350:
ff ff ff ff ff ff 00 1c 02 70 1d 5d 08 00 45 00
01 50 00 01 00 00 ff 11 ba 9c 00 00 00 00 ff ff
ff ff 00 44 00 43 01 3c 98 aa 01 01 06 00 9d cc
a8 c5 00 00 00 00 00 00 00 00 00 00 00 00 00 00
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
03 1c 0c 10 70 61 6e 6f 5f 75 73 62 5f 73 6e 69
66 66 65 72 ff 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00
dhcp_select: REQUESTING
dhcp_select(): set request timeout 2000 msecs
Rx #0: Read 342 (0x156) bytes from Rx Fifo:
00 1c 02 70 1d 5d 4c e6 76 3e f3 1a 08 00 45 00
01 48 89 02 00 00 40 11 78 df c0 a8 7b 01 c0 a8
7b 71 00 43 00 44 01 34 c8 d2 02 01 06 00 9d cc
a8 c5 00 00 00 00 00 00 00 00 c0 a8 7b 71 c0 a8
7b 01 00 00 00 00 00 1c 02 70 1d 5d 00 00 00 00
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
00 00 00 00 00 00 63 82 53 63 35 01 05 36 04 c0
a8 7b 01 33 04 00 00 a8 c0 3a 04 00 00 54 60 3b
04 00 00 93 a8 01 04 ff ff ff 00 1c 04 c0 a8 7b
ff 03 04 c0 a8 7b 01 ff 00 00 00 00 00 00 00 00
00 00 00 00 00 00
dhcp_recv(pbuf = 1b2dc) from DHCP server 192.168.123.1 port 67
pbuf->len = 300
pbuf->tot_len = 300
skipping option 28 in options
searching DHCP_OPTION_MESSAGE_TYPE
DHCP_ACK received
dhcp_bind(netif=187c4) et0
dhcp_bind(): t0 renewal timer 43200 secs
dhcp_bind(): set request timeout 43200000 msecs
dhcp_bind(): t1 renewal timer 21600 secs
dhcp_bind(): set request timeout 21600000 msecs
dhcp_bind(): t2 rebind timer 37800 secs
dhcp_bind(): set request timeout 37800000 msecs
dhcp_bind(): IP: 0x717ba8c0 SN: 0x00ffffff GW: 0x017ba8c0
pano_netif_output: called tot_len: 42, len: 42:
ff ff ff ff ff ff 00 1c 02 70 1d 5d 08 06 00 01
08 00 06 04 00 01 00 1c 02 70 1d 5d c0 a8 7b 71
00 00 00 00 00 00 c0 a8 7b 71
Rx #0: Read 60 (0x3c) bytes from Rx Fifo:
ff ff ff ff ff ff 00 00 00 00 00 00 88 99 03 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00
```

### Determing the Pano's IP address

The DHCP client provides a host name of "pano_usb_sniffer" to the DHCP server
when an IP address is requested.  IF your router provides DNS service for 
local clients you should be able to telnet into your pano by host name. 
I use an OpenWRT based router and it provides this service.

If your router doesn't provide DNS for local clients you will need to use 
determine the Pano's IP address manually.

Currently the assigned IP address is logged as a 32bit hex number so it's 
probably easier to determine the IP address by consulting your router's admin
interface. Failing that search for a line beginnig wiht "dhcp_handle_offer()"

```
dhcp_handle_offer(): offer for 0x717ba8c0
```

To convert the 32 bit string into an IP address:
1. Convert each nibble to decimal.
2. Reverse the byte order.

For my address of 0x717ba8c0:
71 -> 113, 7b 0> 123, a8 -> 168, c0 -> 192 or 192.168.123.113

Here's an example of a long ping test (several hours)

```
skip@dell-790:~/pano/working/panog2_usb_sniffer/pano/fw/lwip$ ping 192.168.123.113
PING 192.168.123.113 (192.168.123.113) 56(84) bytes of data.
64 bytes from 192.168.123.113: icmp_seq=1 ttl=255 time=86.5 ms
64 bytes from 192.168.123.113: icmp_seq=2 ttl=255 time=44.1 ms
...
64 bytes from 192.168.123.113: icmp_seq=7373 ttl=255 time=16.8 ms
64 bytes from 192.168.123.113: icmp_seq=7374 ttl=255 time=21.9 ms
^C
--- 192.168.123.113 ping statistics ---
7374 packets transmitted, 7374 received, 0% packet loss, time 7385766ms
rtt min/avg/max/mdev = 7.235/33.605/393.891/16.467 ms
```

Here's an example of a telnet test

```
skip@dell-790:~/pano/working/panog2_usb_sniffer/pano/fw/lwip$ telnet pano_usb_sniffer
Trying 192.168.123.113...
Connected to pano_usb_sniffer.lan.
Escape character is '^]'.
Welcome to the Pano world via TCP/IP!
This is a test and only a test!
This is a test and only a test!
^[^]
telnet> quit
Connection closed.
```

### Ethernet Status lights

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

