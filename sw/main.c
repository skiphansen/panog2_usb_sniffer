//-----------------------------------------------------------------
//                       USB Sniffer
//                           V0.1
//                     Ultra-Embedded.com
//                       Copyright 2015
//
//               Email: admin@ultra-embedded.com
//
//                       License: LGPL
//-----------------------------------------------------------------
//
// Copyright (C) 2011 - 2013 Ultra-Embedded.com
//
// This source file may be used and distributed without         
// restriction provided that this copyright statement is not    
// removed from the file and that any derivative work contains  
// the original copyright notice and the associated disclaimer. 
//
// This source file is free software; you can redistribute it   
// and/or modify it under the terms of the GNU Lesser General   
// Public License as published by the Free Software Foundation; 
// either version 2.1 of the License, or (at your option) any   
// later version.
//
// This source is distributed in the hope that it will be       
// useful, but WITHOUT ANY WARRANTY; without even the implied   
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      
// PURPOSE.  See the GNU Lesser General Public License for more 
// details.
//
// You should have received a copy of the GNU Lesser General    
// Public License along with this source; if not, write to the 
// Free Software Foundation, Inc., 59 Temple Place, Suite 330, 
// Boston, MA  02111-1307  USA
//-----------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h> 
#include <ftdi.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdbool.h>
#include <errno.h>

#include "log_format.h"
#include "usb_defs.h"
#include "usb_helpers.h"
#include "usb_sniffer.h"
#include "log_file.h"
#include "socket_hw.h"
#include "usb_sniffer_defs.h"

void Usage(void);

//-----------------------------------------------------------------
// user_abort_check
//-----------------------------------------------------------------
static int user_abort_check(void)
{
   struct timeval tv;
   fd_set fds;
   tv.tv_sec = 0;
   tv.tv_usec = 0;
   FD_ZERO(&fds);
   FD_SET(STDIN_FILENO, &fds);
   select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
   return(FD_ISSET(0, &fds));
}
//-----------------------------------------------------------------
// main
//-----------------------------------------------------------------
int main(int argc, char *argv[])
{
   char *filename = "capture.bin";
   int res;
   int c;
   int help = 0;

   int dev_addr = -1;
   int endpoint = -1;
   int disable_sof = 0;
   int disable_in_nak = 0;
   int cont_mode = 1;
   int decode_log_file = 0;
   bool ExcludeEndpoint = false;
   bool ExcludeDevice = false;
   tUsbSpeed speed = USB_SPEED_HS;
   char *ext;
   FILE *fout = NULL;
   bool DataLost = false;
   uint32_t FifoStatus;
   uint32_t Available;
   uint8_t *Buffer = (uint8_t *) malloc(USB_FIFO_SIZE);
   int Ret = -1;
   char *IP = NULL;

   while((c = getopt (argc, argv, "d:D:e:E:slf:nu:iI:")) != -1) {
      switch(c) {
         case 'D': // Device
            ExcludeDevice = true;
            dev_addr = (int)strtoul(optarg, NULL, 0);
            break;

         case 'd': // Device
            ExcludeDevice = false;
            dev_addr = (int)strtoul(optarg, NULL, 0);
            break;

         case 'E': // Endpoint
            ExcludeEndpoint = true;
            endpoint = (int)strtoul(optarg, NULL, 0);
            break;

         case 'e': // Endpoint
            ExcludeEndpoint = false;
            endpoint = (int)strtoul(optarg, NULL, 0);
            break;

         case 's': // Drop SOF
            disable_sof = 1;
            break;

         case '1': // One shot mode (stop on buffer full)
            cont_mode = 0;
            break;

         case 'f': // Filename
            filename = optarg;
            break;

         case 'u': // Speed
            if(strcmp(optarg, "hs") == 0)
               speed = USB_SPEED_HS;
            else if(strcmp(optarg, "fs") == 0)
               speed = USB_SPEED_FS;
            else if(strcmp(optarg, "ls") == 0)
               speed = USB_SPEED_LS;
            else {
               fprintf (stderr,"ERROR: Incorrect speed selection\n");
               help = 1;
            }
            break;

         case 'i':
            disable_in_nak = 1;
            break;

         case 'I': // IP address
            if(IP != NULL) {
               free(IP);
            }
            IP = strdup(optarg);
            break;

         default:
            help = 1;
            break;
      }
   }

   do {
      if(help) {
         Usage();
         break;
      }

      if(Buffer == NULL) {
         fprintf(stderr, "Error: malloc failed\n");
         break;
      }

      // Capture mode
      if((Ret = usb_sniffer_init(IP)) != 0) {
         break;
      }

      // Disable probe
      usb_sniffer_stop();

      // Configure device
      usb_sniffer_match_device(dev_addr, ExcludeDevice);
      usb_sniffer_match_endpoint(endpoint, ExcludeEndpoint);
      usb_sniffer_drop_sof(disable_sof);
      usb_sniffer_drop_in_nak(disable_in_nak);
      usb_sniffer_continuous_mode(0);
      usb_sniffer_set_speed(speed);
      usb_sniffer_set_config();

      uint32_t rd_ptr = 0;

      if((fout = fopen(filename,"wb")) == NULL) {
         fprintf(stderr,"ERROR: couldn't open \"%s\" - %s\n",
                 filename,strerror(errno));
         break;
      }

      // Enable probe
      usb_sniffer_start();

      printf("Sampling: Press <ENTER> to stop\n");
      uint32_t data_count = 0;
      do {
         if(user_abort_check()) {
            break;
         }

         // Get current write pointer
         FifoStatus = usb_sniffer_status();
         if(!DataLost && (FifoStatus & USB_FIFO_STS_ERR_MASK) != 0) {
            DataLost = true;
            printf("Error: data loss\n");
         }

         Available = FifoStatus & USB_FIFO_STS_AVAIL_MASK;

         if(Available > 0) {
            Available *= sizeof(uint32_t);

            // Extract buffer from target
            if(usb_sniffer_read_buffer(Buffer,Available) != Available) {
               break;
            }
            if(fwrite(Buffer,Available,1,fout) != 1) {
               printf("fwrite failed - %s\n",strerror(errno));
               break;
            }

            if((data_count / 1024) != ((data_count + Available) / 1024))
               printf("\r%dKB ", ((data_count + Available) / 1024));
            data_count += Available;
         }
      } while(!DataLost);

      // Close temp file
      fclose(fout);

      // Disable probe
      usb_sniffer_stop();

      usb_sniffer_close();

   } while(false);

   if(Buffer != NULL) {
      free(Buffer);
   }

   return Ret;
}

void Usage()
{
   fprintf (stderr,"Usage:\n");
   fprintf (stderr,"-d 0xnn        - Match only this device ID\n");
   fprintf (stderr,"-D 0xnn        - Exclude this device ID\n");
   fprintf (stderr,"-e 0xnn        - Match only this endpoint\n");
   fprintf (stderr,"-E 0xnn        - Exclude this endpoint\n");
   fprintf (stderr,"-f <filename>  - Capture file (default: capture.bin)\n");
   fprintf (stderr,"-i               Disable IN+NAK capture\n");
   fprintf (stderr,"-I <ip adr>    - IP address of sniffer\n");
   fprintf (stderr,"-s             - Disable SOF collection (breaks timing info)\n");
   fprintf (stderr,"-u ls|fs|hs    - USB speed\n");
   fprintf (stderr,"-1             - One shot mode (stop on single buffer full)\n");
}
