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
#include "hw_interface.h"
#include "ftdi_hw.h"

struct hw_interface_ops *hw_interface_ops = &ftdi_hw_ops;

int hw_interface_init(void)
{
    return hw_interface_ops->interface_init();
}

int hw_interface_close(void)
{
    return hw_interface_ops->interface_close();
}

int hw_mem_write(uint32_t addr, uint8_t *data, int length)
{
    return hw_interface_ops->mem_write(addr, data, length);
}

int hw_mem_read(uint32_t addr, uint8_t *data, int length)
{
    return hw_interface_ops->mem_read(addr, data, length);
}

//-----------------------------------------------------------------
// hw_mem_write_word:
//-----------------------------------------------------------------
int hw_mem_write_word(uint32_t addr, uint32_t data)
{
    uint8_t buffer[4];

    buffer[3] = (data >> 24);
    buffer[2] = (data >> 16);
    buffer[1] = (data >> 8);
    buffer[0] = (data >> 0);

    return hw_mem_write(addr, buffer, 4);
}
//-----------------------------------------------------------------
// hw_mem_read_word:
//-----------------------------------------------------------------
int hw_mem_read_word(uint32_t addr, uint32_t *data)
{
    uint8_t buffer[4];

    int res = hw_mem_read(addr, buffer, 4);
    if (res > 0)
    {
        (*data) = ((uint32_t)buffer[3]) << 24;
        (*data)|= ((uint32_t)buffer[2]) << 16;
        (*data)|= ((uint32_t)buffer[1]) << 8;
        (*data)|= ((uint32_t)buffer[0]) << 0;
    }
    return res;
}
