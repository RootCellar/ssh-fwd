#ifndef UTIL_H
#define UTIL_H

int setup_fd(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  return 1;
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
