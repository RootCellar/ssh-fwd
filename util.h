#ifndef UTIL_H
#define UTIL_H

int setup_fd(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  return 1;
}

#endif
