#include "debug.h"
#include "memory.h"

#include <linux/tcp.h>

#ifndef UTIL_H
#define UTIL_H

// Split num into bytes split1 and split2, which are given by the caller
// and set in this function
void short_to_bytes(short num, char* split1, char* split2) {
  (*split1) = num & 0xFF;
  (*split2) = (num >> 8) & 0xFF;
}

// Return the short represented by split1 and split2
short bytes_to_short(char split1, char split2) {
  short num = 0;

  num = split2;
  num <<= 8;

  num |= split1 & 0xFF;

  return num;
}

// INCOMPLETE
struct buffer_data {
  char* buffer;
  size_t buffer_size;
  size_t amount_filled;
  int complete_message;
};

// INCOMPLETE
// Create and initialize buffer data
struct buffer_data create_buffer_data(size_t size) {
  struct buffer_data toRet;
  toRet.buffer = tMalloc(size);
  if( !(toRet.buffer > 0) ) {
    return toRet;
  }

  toRet.buffer_size = size;
  toRet.amount_filled = 0;
  toRet.complete_message = 0;

  return toRet;
}

// INCOMPLETE
// Read data from a socket into a buffer,
// where the first two bytes of any message is the channel and the next
// two are the length
int channel_read_buffered(int fd, struct buffer_data* buffer) {
  if( !(buffer > 0) ) return 0;
  if( buffer->buffer_size < 8 ) return 0;

  char* read_pos = buffer->buffer + buffer->amount_filled;
  size_t max_count = buffer->buffer_size - buffer->amount_filled;

  int len_read = read(fd, read_pos, max_count);

  if(errno == EAGAIN || len_read == 0) return 0;
  else if(errno != 0) return 0;

  buffer->amount_filled += len_read;

  short channel, len;

  channel = bytes_to_short(buffer->buffer[0], buffer->buffer[1]);
  len = bytes_to_short(buffer->buffer[2], buffer->buffer[3]);

  if(buffer->amount_filled == len + 4) {
    buffer->complete_message = 1;
  }

  return len_read;
}

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
