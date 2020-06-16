#ifndef _socket_H_
#define _socket_H_

#include <stdint.h>

int socket_init(char *IP);
int socket_close(void);
int socket_mem_write(uint32_t addr, uint8_t *data, int length);
int socket_mem_read(uint32_t addr, uint8_t *data, int length);
int socket_mem_write_word(uint32_t addr, uint32_t data);
int socket_mem_read_word(uint32_t addr, uint32_t *data);
int socket_read_fifo(uint8_t *data, int length);

#endif
