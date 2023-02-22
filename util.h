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

#endif
