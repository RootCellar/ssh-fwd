#include <linux/tcp.h>

#include "util.h"

#ifndef INET_H
#define INET_H

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

  memset(toRet.buffer, 0, size);

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
  debug_printf("%d %d", channel, len);

  if(buffer->amount_filled >= len + 4) {
    buffer->complete_message = 1;
  }

  return len_read;
}

int buffer_consume(void* dest, struct buffer_data* buffer, size_t bytes) {
  if(dest == 0 || buffer == 0) return 1;

  if(buffer->buffer_size < bytes) return 1;

  // Copy to destination
  errno = 0;
  memcpy(dest, buffer->buffer, bytes);
  if(errno != 0) return 1;

  // Shift the rest of the buffer back
  // don't use memcpy() because it doesn't necessarily copy 
  // in the correct order (beginning to end)
  for(size_t i = 0; i < buffer->buffer_size - bytes; i++) {
    buffer->buffer[i] = buffer->buffer[bytes + i];
  }

  return 0;
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