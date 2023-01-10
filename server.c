/*
 * Darian Marvel
 * 01/09/2023
 * Server for ssh forwarder
*/

// Headers
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "debug.h"
#include "util.h"

// Constants
#define PORT 8090
#define BUFFER_SIZE 16384
#define CLIENT_LIST_SIZE 1024

// Structures

struct client_data {
  int connection_fd;
  int listen_fd;
};

// Server-specific functions

void handle_client_connections(struct client_data* clients, char* buffer) {
  int len;
  int client_fd;
  for(int i = 0; i < CLIENT_LIST_SIZE; i++) {
    client_fd = clients[i].connection_fd;
    if(client_fd < 0) {
      continue;
    }
    debug_printf("reading %d\n", i);
    errno = 0;
    len = read(client_fd, buffer, BUFFER_SIZE);
    if(errno == EAGAIN) continue;
    else if(len == 0) {
      debug_printf("Client %d disconnected.\n", i);
      close(client_fd);
      if(clients[i].listen_fd > 0) {
        debug_print("Client had forward socket, closing...\n");
        close(clients[i].listen_fd);
      }
      clients[i].connection_fd = -1;
      clients[i].listen_fd = -1;
    }
    else if(errno != 0) {
      perror("error on read");
      exit(EXIT_FAILURE);
    }
    else {
      buffer[len-1] = 0;
      printf("Client 1: %s\n", buffer);
    }
  }
}

int find_empty_client_slot(struct client_data* clients) {
  for(int i = 0; i < CLIENT_LIST_SIZE; i++) {
    if(clients[i].connection_fd <= 0) { return i; }
  }

  return -1;
}

int create_server_socket(int port) {
  int server_fd;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  int opt = 1;
  int result = 0;

  errno = 0;

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(server_fd <= 0) {
    return -1;
  }

  // set socket options
  result = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
  if(result) {
    return -1;
  }

  // Make server socket non-blocking
  setup_fd(server_fd);

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons( PORT );

  result = bind(server_fd, (struct sockaddr *)&address, sizeof(address));
  if(result < 0) { return -1; }

  result = listen(server_fd, 3);
  if(result < 0) { return -1; }

  errno = 0;

  return server_fd;
}

int main(int argc, char const *argv[])
{
  int server_fd;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);
  char buffer[BUFFER_SIZE] = {0};
  char *string;

  struct client_data clients[CLIENT_LIST_SIZE];

  for(int i = 0; i < CLIENT_LIST_SIZE; i++) {
    clients[i].connection_fd = -1;
    clients[i].listen_fd = -1;
  }

  //String constants
  char *clientsFull = "Sorry, no more clients can join!\n";
  char *needOther = "Only one client is connected!\n";

  server_fd = create_server_socket(PORT);
  if(server_fd < 0) {
    perror("Could not create main server socket");
    exit(1);
  }

  int newSocket = -1;
  int client1 = -1;
  int client2 = -1;

  while(1) {

    // Attempt to Accept a connection

    errno = 0;
    newSocket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if(errno == EAGAIN) {
      //continue...
    }
    else if(newSocket >= 0) {

      debug_printf("Accepted a new client connection %d\n", newSocket);
      int spot = find_empty_client_slot(clients);
      if(spot < 0) {
        debug_print("All slots are full!\n");
        send(newSocket, clientsFull, strlen(clientsFull), 0);
        close(newSocket);
      }
      else {
        debug_printf("Found empty spot for client in %d\n", spot);
        setup_fd(newSocket);
        clients[spot].connection_fd = newSocket;

        debug_print("Attempting to make listen socket for client\n");
        int port = PORT + (spot+1);
        int newServerSocket = create_server_socket(port);
        if(newServerSocket < 0) {
          debug_print("Server socket creation failed\n");
        }
        else {
          clients[spot].listen_fd = newServerSocket;
          debug_printf("Client listen socket on %d created.\n", port);
        }
      }

    }
    else {
      perror("accept failed");
      exit(EXIT_FAILURE);
    }

    handle_client_connections(clients, buffer);

    usleep(100000);
  }

  return 0;
  }
