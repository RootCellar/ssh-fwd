#include "debug.h"

#ifndef UTIL_H
#define UTIL_H

unsigned long int BYTES_ALLOCATED = 0;

void* tMalloc(unsigned long int len) {
  debug_printf("Allocating %lu bytes\n", len);

  char* toRet = malloc(len);

  if(toRet > 0) {
    //BYTES_ALLOCATED += len;
    //debug_printf("There are now %lu bytes allocated\n", BYTES_ALLOCATED);
  }

  return toRet;
}

void tFree(void* ptr){
  if(ptr > 0) {
    debug_print("Freeing a pointer\n");
    //debug_printf("%lu %lu\n", sizeof ptr, sizeof *ptr);
    //size_t len = (sizeof ptr) / (sizeof *ptr );
    //debug_printf("Freeing %lu bytes\n", len);
    free(ptr);
    //BYTES_ALLOCATED -= len;
    //debug_printf("There are now %lu bytes allocated\n", BYTES_ALLOCATED);
  }
}

int setup_fd(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  return 1;
}

char* setup_socket(int fd, int bufferSize) {
  debug_print("Setting up a socket...\n");
  setup_fd(fd);

  return tMalloc(sizeof(char) * bufferSize);
}

void forward_data(int dest, char* buffer) {
  write(dest, buffer, strlen(buffer));
  perror("write");
}

void sendData(int fd, char* data, int count) {
  int j;
  int sent = 0;
  while (sent < count) {
      j = write(fd, data + sent, count - sent);

      if(j < 0) {
        perror("write loop\n");
        break;
      }

      //if (j == -1) {
      //    DIE("write"); // TODO is errno EPIPE
      //}

      sent += j;
  }

  //write(fd, data, count);
}

void sendString(int fd, char* data) {
  sendData(fd, data, strlen(data));
  sendData(fd, '\0', 1);
}

#endif
