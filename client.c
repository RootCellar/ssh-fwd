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
  int sock = -1;
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock < 0) {
    return -1;
  }

  struct sockaddr_in serv_addr;

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  debug_print("Have socket, setting up remote address...\n");

  if(inet_pton(AF_INET, host, &serv_addr.sin_addr)<0)
  {
      perror("Invalid address/ Address not supported");
      return -1;
  }

  debug_printf("Connecting to host %s port %d...\n", host, port);

  int result = -1;
  result = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if( result < 0 ) {
    return -1;
  }

  debug_print("Connected, making socket non-blocking...\n");

  int flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL, flags | O_NONBLOCK);

  debug_print("Returning socket file descriptor...\n");

  return sock;

}

int main(int argc, char* argv[])
{

  char* serverAddress = "127.0.0.1";

  if(argc > 1) {
    serverAddress = argv[1];
  }
  printf("Will connect to %s\n", serverAddress);

  int sock = -1; //, valread;
  int fwd_sock = -1;
  //struct sockaddr_in serv_addr;
  //char *string = "hello";
  char buffer[BUFFER_SIZE] = {0};

  sock = create_connection(serverAddress, PORT);
  if(sock < 0) {
    EXIT_FAIL();
  }

  printf("Connected\n");

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
      if(fwd_sock >= 0) {
        close(fwd_sock);
        fwd_sock = -1;
      }
      sock = -1;
    }
    else if (errno != 0) {
      EXIT_FAIL();
    }
    else {
      //printf("From Server: %s\n", buffer);
      if(fwd_sock < 0) {
        // Parse Command
        if(strcmp(buffer,"connect") == 0) {
          debug_print("Connecting to SSH Server...\n");
          // Begin forward
          fwd_sock = create_connection("127.0.0.1", 22);
          if(fwd_sock < 0) {
            printf("Could not initiate connection");
            EXIT_FAIL();
          }
        }
      }
      else {
        sendData(fwd_sock, buffer, len);
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
        EXIT_FAIL();
      }
      else {
        //printf("From SSH Server: %s\n", buffer);
        if(fwd_sock < 0) {

        }
        else {
          sendData(sock, buffer, len);
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
