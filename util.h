#include "debug.h"
#include "memory.h"

#include <linux/tcp.h>

#ifndef UTIL_H
#define UTIL_H

int setup_fd(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  flags = 1;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *) &flags, sizeof(int));

  return 1;
}

char* setup_socket(int fd, int bufferSize) {
  debug_print("Setting up a socket...\n");
  setup_fd(fd);

  return tMalloc(sizeof(char) * bufferSize);
}

void sendData(int fd, char* data, int count) {
  int j;
  int sent = 0;
  while (sent < count) {
      j = write(fd, data + sent, count - sent);

      if(j >= 0) sent += j;

      if(j < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        perror("write loop");
        break;
      }
  }
}

void sendString(int fd, char* data) {
  sendData(fd, data, strlen(data));
}

#endif
