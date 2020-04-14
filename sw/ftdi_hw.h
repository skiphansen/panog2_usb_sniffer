#ifndef _FTDI_HW_H_
#define _FTDI_HW_H_

#include <stdint.h>
#include "hw_interface.h"

extern struct hw_interface_ops ftdi_hw_ops;

//-----------------------------------------------------------------
// Defines:
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Prototypes:
//-----------------------------------------------------------------
int ftdi_hw_init(void);
int ftdi_hw_close(void);

// Memory Access
int ftdi_hw_mem_write(uint32_t addr, uint8_t *data, int length);
int ftdi_hw_mem_read(uint32_t addr, uint8_t *data, int length);

// GPIO
int ftdi_hw_gpio_write(uint8_t value);
int ftdi_hw_gpio_read(uint8_t *value);

#endif
