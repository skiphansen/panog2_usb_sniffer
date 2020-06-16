#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include "socket_hw.h"

#define CMD_NOP        0x0
#define CMD_WR         0x1
#define CMD_RD         0x2
#define CMD_RD_FIFO    0x3

static int sock = -1;

int socket_init(char *IP)
{
   struct sockaddr_in addr;
   char *ip_adr;

   ip_adr = IP != NULL ? IP : "127.0.0.1";
   sock = socket(AF_INET, SOCK_STREAM, 0);
   if(sock == -1) {
      fprintf(stderr, "Could not create socket\n");
      return -1;
   }
   addr.sin_addr.s_addr = inet_addr(ip_adr);
   addr.sin_family = AF_INET;
   addr.sin_port = htons(12345);
   if(connect(sock , (struct sockaddr *)&addr , sizeof(addr)) < 0) {
      fprintf(stderr, "Connect to %s failed, ",ip_adr);
      perror("error");
      return -1;
   }
}

int socket_close(void)
{
   close(sock);
   sock = -1;
}

int socket_mem_write(uint32_t addr, uint8_t *data, int length)
{
   uint8_t cmd = CMD_WR;
   if(send(sock, &cmd, sizeof(cmd), 0) < 0) {
      perror("Send failed");
      return -1;
   }

   if(send(sock, &addr, sizeof(addr), 0) < 0) {
      perror("Send failed");
      return -1;
   }

   if(send(sock, &length, sizeof(length), 0) < 0) {
      perror("Send failed");
      return -1;
   }

   if(send(sock, data, length, 0) < 0) {
      perror("Send failed");
      return -1;
   }

   return length;
}

int _socket_mem_read(uint32_t addr, uint8_t *data, int length,uint8_t cmd)
{
   int RxBytes;;
   if(send(sock, &cmd, sizeof(cmd), 0) < 0) {
      perror("Send failed");
      return -1;
   }

   if(cmd == CMD_RD) {
      if(send(sock, &addr, sizeof(addr), 0) < 0) {
         perror("Send failed");
         return -1;
      }
   }

   if(send(sock, &length, sizeof(length), 0) < 0) {
      perror("Send failed");
      return -1;
   }

   printf("Calling recv for %d bytes\n",length);
   if((RxBytes = recv(sock, data, length, MSG_WAITALL)) < 0) {
      perror("Receive failed");
      return -1;
   }
   printf("Bytes received: %d\n",RxBytes);

   return length;
}

int socket_mem_read(uint32_t addr, uint8_t *data, int length)
{
   return _socket_mem_read(addr,data,length,CMD_RD);
}

int socket_read_fifo(uint8_t *data, int length)
{
   return _socket_mem_read(0,data,length,CMD_RD_FIFO);
}


//-----------------------------------------------------------------
// socket_mem_write_word:
//-----------------------------------------------------------------
int socket_mem_write_word(uint32_t addr, uint32_t data)
{
   uint8_t buffer[4];

   buffer[3] = (data >> 24);
   buffer[2] = (data >> 16);
   buffer[1] = (data >> 8);
   buffer[0] = (data >> 0);

   return socket_mem_write(addr, buffer, 4);
}
//-----------------------------------------------------------------
// socket_mem_read_word:
//-----------------------------------------------------------------
int socket_mem_read_word(uint32_t addr, uint32_t *data)
{
   uint8_t buffer[4];

   printf("%s: called\n",__FUNCTION__);
   int res = socket_mem_read(addr, buffer, 4);
   if(res > 0) {
      (*data) = ((uint32_t)buffer[3]) << 24;
      (*data)|= ((uint32_t)buffer[2]) << 16;
      (*data)|= ((uint32_t)buffer[1]) << 8;
      (*data)|= ((uint32_t)buffer[0]) << 0;
      printf("%s: read 0x%x\n",__FUNCTION__,*data);

   }
   return res;
}

