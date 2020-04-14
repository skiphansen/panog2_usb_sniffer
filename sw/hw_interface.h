#ifndef _HW_INTERFACE_H_
#define _HW_INTERFACE_H_

#include <stdint.h>

struct hw_interface_ops {
    int (*interface_init)(void);
    int (*interface_close)(void);
    int (*mem_write)(uint32_t addr, uint8_t *data, int length);
    int (*mem_read)(uint32_t addr, uint8_t *data, int length);
    char *host;
};

//-----------------------------------------------------------------
// Prototypes:
//-----------------------------------------------------------------
int hw_interface_init(void);
int hw_interface_close(void);

// Memory Access
int hw_mem_write(uint32_t addr, uint8_t *data, int length);
int hw_mem_read(uint32_t addr, uint8_t *data, int length);
int hw_mem_write_word(uint32_t addr, uint32_t data);
int hw_mem_read_word(uint32_t addr, uint32_t *data);

extern struct hw_interface_ops *hw_interface_ops;

#endif
