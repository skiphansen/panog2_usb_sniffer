#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include "socket_hw.h"

#define CMD_NOP        0x0
#define CMD_WR         0x1
#define CMD_RD         0x2

static int sock = -1;
struct hw_interface_ops socket_hw_ops;

int socket_hw_init(void)
{
    struct sockaddr_in addr;
    char *ip_adr;

    ip_adr = socket_hw_ops.host != NULL ? socket_hw_ops.host : "127.0.0.1";
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        fprintf(stderr, "Could not create socket\n");
        return -1;
    }
    addr.sin_addr.s_addr = inet_addr(ip_adr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    if (connect(sock , (struct sockaddr *)&addr , sizeof(addr)) < 0) {
        fprintf(stderr, "Connect to %s failed, ",ip_adr);
        perror("error");
        return -1;
    }
}

int socket_hw_close(void)
{
    close(sock);
    sock = -1;
}

int socket_hw_mem_write(uint32_t addr, uint8_t *data, int length)
{
    uint8_t cmd = CMD_WR;
    if (send(sock, &cmd, sizeof(cmd), 0) < 0) {
        perror("Send failed");
        return -1;
    }

    if (send(sock, &addr, sizeof(addr), 0) < 0) {
        perror("Send failed");
        return -1;
    }

    if (send(sock, &length, sizeof(length), 0) < 0) {
        perror("Send failed");
        return -1;
    }

    if (send(sock, data, length, 0) < 0) {
        perror("Send failed");
        return -1;
    }

    return length;
}

int socket_hw_mem_read(uint32_t addr, uint8_t *data, int length)
{
    uint8_t cmd = CMD_RD;
    if (send(sock, &cmd, sizeof(cmd), 0) < 0) {
        perror("Send failed");
        return -1;
    }

    if (send(sock, &addr, sizeof(addr), 0) < 0) {
        perror("Send failed");
        return -1;
    }

    if (send(sock, &length, sizeof(length), 0) < 0) {
        perror("Send failed");
        return -1;
    }

    if (recv(sock, data, length, MSG_WAITALL) < 0) {
        perror("Receive failed");
        return -1;
    }

    return length;
}

struct hw_interface_ops socket_hw_ops = {
    .interface_init = socket_hw_init,
    .interface_close = socket_hw_close,
    .mem_write = socket_hw_mem_write,
    .mem_read = socket_hw_mem_read
};
