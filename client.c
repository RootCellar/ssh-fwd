/*
 * Darian Marvel
 * 01/09/2023
 * Client for ssh forwarder
*/

// Headers
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "debug.h"
#include "util.h"

// Constants
#define PORT 8090
#define SSH_PORT 22
#define BUFFER_SIZE 4096


int create_connection(char* host, int port) {
  int sock;
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock < 0) {
    return -1;
  }

  struct sockaddr_in serv_addr;

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SSH_PORT);

  if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
  {
      perror("Invalid address/ Address not supported");
      return -1;
  }

  int result = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if( result < 0 ) {
    return -1;
  }

  int flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL, flags | O_NONBLOCK);

  return sock;

}

int main(int argc, char const *argv[])
{
    int sock = -1, valread;
    int fwd_sock = -1;
    struct sockaddr_in serv_addr;
    char *string = "hello";
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    debug_print("Connecting to host\n");

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection Failed");
        return -1;
    }

    debug_print("Connected\n");

    // Make stdin non-blocking
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    // Make the socket non-blocking
    flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    while(1) {

      // Reset Error, attempt to read data from the socket

      errno = 0;
      int len = read(sock, buffer, BUFFER_SIZE);

      if(errno == EAGAIN) {
        // No data, continue...
      }
      else if(len == 0) {
        // socket is disconnected
        printf("Disconnected\n");
        close(sock);
        sock = -1;
      }
      else if (errno != 0) {
        // Some other error occurred
        perror("error on read");
        exit(EXIT_FAILURE);
      }
      else {
        printf("From Server: %s\n", buffer);
        if(fwd_sock < 0) {
          // Parse Command
          if(strcmp(buffer,"connect") == 0) {
            debug_print("Connecting to SSH Server...\n");
            // Begin forward
            fwd_sock = create_connection("127.0.0.1", 22);
            if(fwd_sock < 0) {
              printf("Could not initiate connection");
              exit(EXIT_FAILURE);
            }
          }
        }
        else {
          write(fwd_sock, buffer, len);
        }
      }

      if(fwd_sock >= 0) {

        errno = 0;
        int len = read(fwd_sock, buffer, BUFFER_SIZE);

        if(errno == EAGAIN) {
          // No data, continue...
        }
        else if(len == 0) {
          // socket is disconnected
          printf("Disconnected\n");
          close(fwd_sock);
          fwd_sock = -1;
        }
        else if (errno != 0) {
          // Some other error occurred
          perror("error on read");
          exit(EXIT_FAILURE);
        }
        else {
          printf("From SSH Server: %s\n", buffer);
          if(fwd_sock < 0) {

          }
          else {
            write(sock, buffer, len);
          }
        }

      }

      // Reset Error, attempt to read from STDIN

      /*

      errno = 0;
      len = read(STDIN_FILENO, buffer, BUFFER_SIZE);

      if(errno == EAGAIN) {
        // No input, continue...
      }
      else {
        // Send input from STDIN to server
        buffer[len] = 0;
        send(sock, buffer, strlen(buffer), 0);
      }

      */

      usleep(1000);
    }

    return 0;
}
