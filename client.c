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

// Constants
#define PORT 8090


int main(int argc, char const *argv[])
{
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *string = "hello";
    char buffer[2048] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    // Make stdin non-blocking
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    // Make the socket non-blocking
    flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    while(1) {

      // Reset Error, attempt to read data from the socket

      errno = 0;
      int len = read(sock, buffer, 2048);

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
        // Print received data
        buffer[len] = 0;
        printf("From Server: %s\n", buffer);
      }

      // Reset Error, attempt to read from STDIN

      errno = 0;
      len = read(STDIN_FILENO, buffer, 2048);

      if(errno == EAGAIN) {
        // No input, continue...
      }
      else {
        // Send input from STDIN to server
        buffer[len] = 0;
        send(sock, buffer, strlen(buffer), 0);
      }

      usleep(1000);
    }

    return 0;
}
