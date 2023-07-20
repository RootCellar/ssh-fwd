/*
 * Darian Marvel
 * 01/09/2023
 * Server for ssh forwarder
*/

// Headers

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Internet 

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Utility

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

// Local Headers

#include "debug.h"
#include "util.h"
#include "inet.h"

// Constants

#define PORT 10090
#define BUFFER_SIZE 16384
#define CLIENT_LIST_SIZE 32

// Structures

struct client_data {
  int connection_fd;
  int listen_fd;
  int forwarded_fd;
  int wait;
  char* connection_buffer;
  char* forward_buffer;
};

// Server-specific functions

void close_fd(struct client_data* clients, int id) {

  // Close any open connections

  if(clients[id].connection_fd > 0) {
    debug_printf("Disconnecting client %d...", id);
    close(clients[id].connection_fd);
  }
  if(clients[id].listen_fd > 0) {
    debug_print("Client had forward socket, closing...");
    close(clients[id].listen_fd);
  }
  if(clients[id].forwarded_fd > 0) {
    debug_print("Client had forwarded connection, closing...");
    close(clients[id].forwarded_fd);
  }

  // Make sure to mark connections as closed/unused

  clients[id].connection_fd = -1;
  clients[id].listen_fd = -1;
  clients[id].forwarded_fd = -1;

  // Free buffers and set them to null pointers

  if(clients[id].connection_buffer > 0) {
    debug_print("Freeing message buffer...");
    tFree(clients[id].connection_buffer);
    clients[id].connection_buffer = 0;
  }

  if(clients[id].forward_buffer > 0) {
    debug_print("Freeing forward buffer...");
    tFree(clients[id].forward_buffer);
    clients[id].forward_buffer = 0;
  }
}

void close_forward(struct client_data* clients, int id) {
  debug_printf("Closing forwarded socket for client %d", id);
  if(clients[id].forwarded_fd > 0) {
    close(clients[id].forwarded_fd);
  }

  if(clients[id].forward_buffer > 0) {
    debug_print("Freeing forward buffer...");
    tFree(clients[id].forward_buffer);
    clients[id].forward_buffer = 0;
  }

  clients[id].forwarded_fd = -1;
}

/*
 * Forwards data back and forth from the client and the remote connected to it
*/
void handle_client_connections(struct client_data* clients) {
  int len;
  int client_fd, client_forwarded_fd;
  char* buffer;
  for(int i = 0; i < CLIENT_LIST_SIZE; i++) {

    client_fd = clients[i].connection_fd;
    if(client_fd < 0) continue;

    if(clients[i].wait > 0) {
      clients[i].wait--;
      continue;
    }

    buffer = clients[i].connection_buffer;
    errno = 0;
    len = read(client_fd, buffer, BUFFER_SIZE);
    if(errno == EAGAIN) {
      // Nothing
    }
    else if(len == 0) {
      debug_printf("Client %d disconnected.", i);
      close_fd(clients, i);
    }
    else if(errno != 0) {
      debug_printf("Client %d disconnected.", i);
      close_fd(clients, i);
    }
    else {
      client_forwarded_fd = clients[i].forwarded_fd;
      if(client_forwarded_fd >= 0) {
        sendData(client_forwarded_fd, buffer, len);
      }
    }

    client_forwarded_fd = clients[i].forwarded_fd;
    if(client_forwarded_fd < 0) continue;

    buffer = clients[i].forward_buffer;
    errno = 0;
    len = read(client_forwarded_fd, buffer, BUFFER_SIZE);
    if(errno == EAGAIN) {
      // Nothing
    }
    else if(len == 0) {
      debug_printf("Client %d disconnected from forward.", i);
      close_fd(clients, i);
    }
    else if(errno != 0) {
      debug_printf("Client %d disconnected.", i);
      close_fd(clients, i);
    }
    else {
      sendData(client_fd, buffer, len);
    }

  }
}

int find_empty_client_slot(struct client_data* clients) {
  for(int i = 0; i < CLIENT_LIST_SIZE; i++) {
    if(clients[i].connection_fd <= 0) { return i; }
  }

  return -1;
}

int create_server_socket(int port, bool isServerSocket) {
  int server_fd;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  int opt = 1;
  int result = 0;

  errno = 0;

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(server_fd <= 0) {
    perror("Could not create server socket");
    return -1;
  }

  // set socket options
  result = setsockopt(server_fd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
  if(result) {
    perror("Could not create server socket");
    return -1;
  }

  // Make server socket non-blocking
  setup_fd(server_fd);

  address.sin_family = AF_INET;
  if(isServerSocket) address.sin_addr.s_addr = INADDR_ANY;
  else address.sin_addr.s_addr = inet_addr("127.0.0.1");
  address.sin_port = htons( port );

  debug_print("Binding server socket...");

  if(isServerSocket) {
    result = -1;
    while(result < 0) {
      result = bind(server_fd, (struct sockaddr *)&address, sizeof(address));
      usleep(1000000);
    }
  }
  else {
    result = bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    perror("bind");
  }

  if(result < 0) { return -1; }

  debug_print("Enabling listen...");

  result = listen(server_fd, 3);
  if(result < 0) { return -1; }

  errno = 0;

  return server_fd;
}

void sigHandler(int sigNum)
{
  if(sigNum == SIGINT) {
    printf("\n\nReceived SIGINT\n\n");
    exit(0);
  }
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
    clients[i].forwarded_fd = -1;

    clients[i].connection_buffer = 0;
    clients[i].forward_buffer = 0;
  }

  server_fd = create_server_socket(PORT, true);
  if(server_fd < 0) {
    EXIT_FAIL();
  }

  int newSocket = -1;
  char *clientsFull = "Sorry, no more clients can join!\n";

  signal(SIGINT, sigHandler);

  tResize(65536);

  while(1) {

    // Attempt to Accept a connection

    errno = 0;
    newSocket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if(errno == EAGAIN) {
      //continue...
    }
    else if(errno != 0) {
      EXIT_FAIL();
    }
    else if(newSocket >= 0) {

      debug_printf("Accepted a new client connection %d", newSocket);
      int spot = find_empty_client_slot(clients);
      if(spot < 0) {
        debug_print("All slots are full!");
        send(newSocket, clientsFull, strlen(clientsFull), 0);
        close(newSocket);
      }
      else {
        debug_printf("Found empty spot for client in %d", spot);
        clients[spot].connection_buffer = setup_socket(newSocket, BUFFER_SIZE);
        clients[spot].connection_fd = newSocket;
        if(clients[spot].connection_buffer <= 0) {
          debug_print("Could not allocate buffer space for the client");
          close_fd(clients, spot);
        }
        else {

          debug_print("Attempting to make listen socket for client");
          int port = PORT + (spot+1);
          int newServerSocket = create_server_socket(port, true);
          if(newServerSocket < 0) {
            debug_print("Server socket creation failed");
            close_fd(clients, spot);
          }
          else {
            clients[spot].listen_fd = newServerSocket;
            debug_printf("Client listen socket on %d created.", port);
          }

        }
      }

    }
    else {
      EXIT_FAIL();
    }

    // Accept forwarded connections for each client's server socket

    int client_fd;
    for(int i = 0; i < CLIENT_LIST_SIZE; i++) {
      errno = 0;
      client_fd = clients[i].listen_fd;
      if(client_fd < 0) continue;
      newSocket = accept(client_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
      if(errno == EAGAIN) {
        //continue...
      }
      else if(errno != 0) {
        EXIT_FAIL();
      }
      else if(newSocket >= 0) {

        if(clients[i].forwarded_fd < 0) {
          clients[i].forwarded_fd = newSocket;
          debug_printf("Accepted a new forwarded connection %d", newSocket);

          clients[i].forward_buffer = setup_socket(newSocket, BUFFER_SIZE);
          if(clients[i].forward_buffer <= 0) {
            debug_print("Could not create buffer for forward connection");
            close_fd(clients, i);
            continue;
          }

          sendString(clients[i].connection_fd, "connect");
          clients[i].wait = 1000;
        }
        else {
          close(newSocket);
          debug_print("Rejected a forwarded connection");
        }

      }
      else {
        EXIT_FAIL();
      }
    }

    handle_client_connections(clients);

    usleep(1000);
  }

  return 0;
}
