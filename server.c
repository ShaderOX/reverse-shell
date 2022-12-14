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
#define BUFFER_SIZE 4096

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

  printf("Created and bound socket.\n");

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

    printf("Connected to %s\n\n", inet_ntoa(client_addr.sin_addr));

    char serverIp[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(socket_addr.sin_addr), serverIp, INET_ADDRSTRLEN);

    if (send(r_socket, serverIp, strlen(serverIp), 0) < 0)
    {
      ERROR_AND_EXIT("Couldn't send data to client.");
    }

    char *receive_buffer = calloc(BUFFER_SIZE, sizeof(char));
    char *send_buffer = calloc(BUFFER_SIZE, sizeof(char));
    FILE *outfp;

    for (;;)
    {
      memset(receive_buffer, 0, BUFFER_SIZE);
      int bytes_received = recv(r_socket, receive_buffer, BUFFER_SIZE, 0);
      if (bytes_received < 0)
      {
        ERROR_AND_EXIT("Couldn't receive data from client.");
      }
      printf("Received `%s` command\n", receive_buffer);

      printf("Executing command...\n");
      outfp = popen(receive_buffer, "r");
      if (outfp == NULL)
      {
        ERROR_AND_EXIT("Couldn't open file.");
      }
      printf("Dumping result to temporary file...\n");

      printf("Reading temporary file...\n");
      memset(send_buffer, 0, BUFFER_SIZE);
      char c = 0;
      for (size_t counter = 0;
           counter < BUFFER_SIZE;
           counter++)
      {
        if ((c = fgetc(outfp)) == EOF)
        {
          break;
        }
        send_buffer[counter] = c;
      }

      printf("Sending result to client...\n");
      if (send(r_socket, send_buffer, BUFFER_SIZE, 0) < 0)
      {
        ERROR_AND_EXIT("Couldn't send data to client.");
      }
      printf("\n");
    }

    fclose(outfp);
    close(r_socket);
    free(receive_buffer);
    free(send_buffer);
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
