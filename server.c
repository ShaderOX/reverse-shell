#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define ERROR(message)                 \
  {                                    \
    printf("ERROR: %s\n\n", #message); \
  }
#define ERROR_AND_EXIT(message) \
  {                             \
    ERROR(message);             \
    exit(EXIT_FAILURE);         \
  }

#define BACKLOG 16
#define BUFFER_SIZE 1024

int s_socket;

void get_command_line_arguments(int argc, char *argv[], uint16_t *port);
void handle_sigint();

int main(int argc, char *argv[])
{
  // Taking SIGINT into account.
  signal(SIGINT, handle_sigint);

  uint16_t port;
  get_command_line_arguments(argc, argv, &port);

  int l_socket;

  struct sockaddr_in socket_addr = {
      .sin_family = AF_INET,
      .sin_addr.s_addr = htonl(INADDR_ANY),
      .sin_port = htons(port)};

  // Creating the socket
  if ((s_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    ERROR_AND_EXIT("Socket unable to create a socket.");
  }

  // Creating listening socket
  if ((l_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    ERROR_AND_EXIT("Couldn't create listening socket.");
  }

  // Binding the socket
  if ((bind(l_socket, (struct sockaddr *)&socket_addr, (socklen_t)sizeof(socket_addr))) < 0)
  {
    ERROR_AND_EXIT("Couldn't bind socket.");
  }

  printf("Created and binded socket.\n");

  if ((listen(l_socket, BACKLOG)) < 0)
  {
    ERROR_AND_EXIT("Couldn't open socket for listening connections");
  }

  struct sockaddr_in client_addr;
  socklen_t client_addr_length;

  printf("Waiting for connections at %s:%d...\n", inet_ntoa(socket_addr.sin_addr), ntohs(socket_addr.sin_port));
  for (;;)
  {
    int r_socket = -1;
    if ((r_socket = accept(l_socket, (struct sockaddr *)&client_addr, &client_addr_length)) < 0)
    {
      ERROR_AND_EXIT("Couldn't accept connection.");
    }

    printf("Connected to %s\n", inet_ntoa(client_addr.sin_addr));

    char serverIp[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(socket_addr.sin_addr), serverIp, INET_ADDRSTRLEN);

    if (send(r_socket, serverIp, strlen(serverIp), 0) < 0)
    {
      ERROR_AND_EXIT("Couldn't send data to client.");
    }

    close(r_socket);
  }
}

void get_command_line_arguments(int argc, char *argv[], uint16_t *port)
{
  if (argc != 2)
  {
    printf("Usage: %s <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  *port = atoi(argv[1]);
}

void handle_sigint()
{
  printf("Shutting down...\n");
  shutdown(s_socket, SHUT_RDWR);

  exit(EXIT_SUCCESS);
}
