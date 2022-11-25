#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define ERROR_AND_EXIT(message)        \
  {                                    \
    printf("ERROR: %s\n\n", #message); \
    exit(EXIT_FAILURE);                \
  }

#define BUFFER_SIZE 4096

void get_command_line_arguments(int argc, char *argv[], uint16_t *port);

int main(int argc, char *argv[])
{
  uint16_t port;
  get_command_line_arguments(argc, argv, &port);

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_pton(AF_INET, "0.0.0.0", &server_addr.sin_addr);

  printf("Trying to connect to the server at %s:%d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

  int socket_fd;
  if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    ERROR_AND_EXIT("Socket unable to create a socket.");
  }

  if ((connect(socket_fd, (struct sockaddr *)&server_addr, (socklen_t)sizeof(server_addr))) < 0)
  {
    ERROR_AND_EXIT("Couldn't connect to server.");
  }

  printf("Connected to server.\n");

  char *receive_buffer = calloc(BUFFER_SIZE, sizeof(char));
  char *send_buffer = calloc(BUFFER_SIZE, sizeof(char));
  char serverIp[INET_ADDRSTRLEN];

  // Get server ip and use it as prompt
  if ((recv(socket_fd, receive_buffer, BUFFER_SIZE, 0)) < 0)
  {
    ERROR_AND_EXIT("Couldn't receive data from server.");
  }
  strncpy(serverIp, receive_buffer, INET_ADDRSTRLEN);

  printf("Server IP: %s\n", serverIp);

  close(socket_fd);
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
