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
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#include "usb_defs.h"
#include "log_format.h"
#include "usb_helpers.h"
#include "usb_sniffer.h"
#include "usb_sniffer_defs.h"
#include "socket_hw.h"

//-----------------------------------------------------------------
// Defines
//-----------------------------------------------------------------
#define CHUNK_SIZE           2048

//-----------------------------------------------------------------
// Locals
//-----------------------------------------------------------------
static uint32_t _mem_base = 0;
static uint32_t _mem_size = 0;
static uint32_t _cfg_reg  = 0;

//-----------------------------------------------------------------
// usb_sniffer_init
//-----------------------------------------------------------------
int usb_sniffer_init(char *IP)
{
   return socket_init(IP);
}
//-----------------------------------------------------------------
// usb_sniffer_close
//-----------------------------------------------------------------
int usb_sniffer_close(void)
{
    return socket_close();
}
//-----------------------------------------------------------------
// usb_sniffer_match_device
//-----------------------------------------------------------------
int usb_sniffer_match_device(int dev, bool exclude)
{
    _cfg_reg &= ~(USB_BUFFER_CFG_DEV_MASK << USB_BUFFER_CFG_DEV_SHIFT);
    _cfg_reg &= ~(USB_BUFFER_CFG_MATCH_DEV_MASK << USB_BUFFER_CFG_MATCH_DEV_SHIFT);
    _cfg_reg &= ~(USB_BUFFER_CFG_EXCLUDE_DEV_MASK << USB_BUFFER_CFG_EXCLUDE_DEV_SHIFT);

    if (dev >= 0)
    {
        _cfg_reg |= (dev << USB_BUFFER_CFG_DEV_SHIFT);

        if (exclude)
            _cfg_reg |= (1   << USB_BUFFER_CFG_EXCLUDE_DEV_SHIFT);
        else
            _cfg_reg |= (1   << USB_BUFFER_CFG_MATCH_DEV_SHIFT);
    }

    return 0;
}
//-----------------------------------------------------------------
// usb_sniffer_match_endpoint
//-----------------------------------------------------------------
int usb_sniffer_match_endpoint(int ep, bool exclude)
{
    _cfg_reg &= ~(USB_BUFFER_CFG_EP_MASK << USB_BUFFER_CFG_EP_SHIFT);
    _cfg_reg &= ~(USB_BUFFER_CFG_MATCH_EP_MASK << USB_BUFFER_CFG_MATCH_EP_SHIFT);
    _cfg_reg &= ~(USB_BUFFER_CFG_EXCLUDE_EP_MASK << USB_BUFFER_CFG_EXCLUDE_EP_SHIFT);

    if (ep >= 0)
    {
        _cfg_reg |= (ep << USB_BUFFER_CFG_EP_SHIFT);

        if (exclude)
            _cfg_reg |= (1  << USB_BUFFER_CFG_EXCLUDE_EP_SHIFT);
        else
            _cfg_reg |= (1  << USB_BUFFER_CFG_MATCH_EP_SHIFT);
    }

    return 0;
}
//-----------------------------------------------------------------
// usb_sniffer_drop_sof
//-----------------------------------------------------------------
int usb_sniffer_drop_sof(int enable)
{
    if (enable)
        _cfg_reg |= (USB_BUFFER_CFG_IGNORE_SOF_MASK << USB_BUFFER_CFG_IGNORE_SOF_SHIFT);
    else
        _cfg_reg &= ~(USB_BUFFER_CFG_IGNORE_SOF_MASK << USB_BUFFER_CFG_IGNORE_SOF_SHIFT);

    return 0;
}

int usb_sniffer_drop_in_nak(int enable)
{
    if (enable)
        _cfg_reg |= (USB_BUFFER_CFG_IGNORE_IN_NAK_MASK << USB_BUFFER_CFG_IGNORE_IN_NAK_SHIFT);
    else
        _cfg_reg &= ~(USB_BUFFER_CFG_IGNORE_IN_NAK_MASK << USB_BUFFER_CFG_IGNORE_IN_NAK_SHIFT);

    return 0;
}

//-----------------------------------------------------------------
// usb_sniffer_continuous_mode
//-----------------------------------------------------------------
int usb_sniffer_continuous_mode(int enable)
{
    if (enable)
        _cfg_reg |= (USB_BUFFER_CFG_CONT_MASK << USB_BUFFER_CFG_CONT_SHIFT);
    else
        _cfg_reg &= ~(USB_BUFFER_CFG_CONT_MASK << USB_BUFFER_CFG_CONT_SHIFT);

    return 0;
}
//-----------------------------------------------------------------
// usb_sniffer_set_speed
//-----------------------------------------------------------------
int usb_sniffer_set_speed(tUsbSpeed speed)
{
    _cfg_reg &= ~(USB_BUFFER_CFG_SPEED_MASK << USB_BUFFER_CFG_SPEED_SHIFT);
    _cfg_reg |= (speed << USB_BUFFER_CFG_SPEED_SHIFT);

    return 0;
}

int usb_sniffer_set_config(void)
{
   return socket_mem_write_word(USB_BASE + USB_BUFFER_CFG, _cfg_reg);
}

//-----------------------------------------------------------------
// usb_sniffer_start
//-----------------------------------------------------------------
int usb_sniffer_start(void)
{
    _cfg_reg |= (USB_BUFFER_CFG_ENABLED_MASK << USB_BUFFER_CFG_ENABLED_SHIFT);
    return socket_mem_write_word(USB_BASE + USB_BUFFER_CFG, _cfg_reg);
}
//-----------------------------------------------------------------
// usb_sniffer_stop
//-----------------------------------------------------------------
int usb_sniffer_stop(void)
{
    _cfg_reg &= ~(USB_BUFFER_CFG_ENABLED_MASK << USB_BUFFER_CFG_ENABLED_SHIFT);
    return socket_mem_write_word(USB_BASE + USB_BUFFER_CFG, _cfg_reg);
}
//-----------------------------------------------------------------
// usb_sniffer_triggered
//-----------------------------------------------------------------
int usb_sniffer_triggered(void)
{
    uint32_t status = 0;

    if (socket_mem_read_word(USB_BASE + USB_BUFFER_STS, &status) != sizeof(uint32_t))
    {
        fprintf(stderr, "ERROR: Failed to read status\n");
        return 0;
    }

    if (status & (1 << USB_BUFFER_STS_TRIG_SHIFT))
        return 1;
    else
        return 0;
}
//-----------------------------------------------------------------
// usb_sniffer_overrun
//-----------------------------------------------------------------
int usb_sniffer_overrun(void)
{
    uint32_t status = 0; 

    if (socket_mem_read_word(USB_BASE + USB_BUFFER_STS, &status) != sizeof(uint32_t))
    {
        fprintf(stderr, "ERROR: Failed to read status\n");
        return 0;
    }

    if (status & (1 << USB_BUFFER_STS_DATA_LOSS))
        return 1;
    else
        return 0;
}
//-----------------------------------------------------------------
// usb_sniffer_available
//-----------------------------------------------------------------
uint32_t usb_sniffer_status()
{
    uint32_t Available = 0;
    if (socket_mem_read_word(USB_BASE + USB_FIFO_STS, &Available) != sizeof(uint32_t))
    {
        fprintf(stderr, "ERROR: Failed to read available\n");
    }

    return Available;
}
//-----------------------------------------------------------------
// usb_sniffer_read_buffer
//-----------------------------------------------------------------
int usb_sniffer_read_buffer(uint8_t *buffer, int size)
{
   int Ret = socket_read_fifo(buffer,size);
   if(Ret != size) {
      fprintf(stderr, "Download: reading FIFO data\n");
   }

   return Ret;
}


