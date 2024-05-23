#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int listen_fd;

struct addrinfo *bind_address;
struct addrinfo hints;
//hints.ai_family = AF_INET;


void setup_sockets()
{

  hints.ai_socktype = SOCK_STREAM;
  int rv;
  rv = getaddrinfo("192.168.1.10", "5000", &hints, &bind_address);
  // rv = getaddrinfo("127.0.0.1", "5000", &hints, &bind_address);

  if (rv != 0)
  {
    perror("Could not get requested address.");
    exit(1);
  }

  listen_fd = socket(bind_address->ai_family , bind_address->ai_socktype, bind_address->ai_protocol);
  if (listen_fd < 0)
  {
    perror("Could not create socket.");
    exit(1);
  }

  // Forcefully attaching socket to the port
  // This allows the server to bind to an address that is in the TIME_WAIT state.
  int opt = 1;
  if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
      perror("setsockopt");
      exit(EXIT_FAILURE);
  }

  rv = bind(listen_fd, bind_address->ai_addr, bind_address->ai_addrlen);

  if (rv < 0)
  {
    perror("Could not bind to socket.");
    exit(1);
  }

  freeaddrinfo(bind_address);

  rv = listen(listen_fd, 10);
  if (rv < 0)
  {
    perror("Could not listen to socket.");
    exit(1);
  }

}

int main()
{
  setup_sockets();

  struct sockaddr_storage client_address;
  socklen_t client_len = sizeof(client_address);
  // Wait for connection
  int client_fd = accept(listen_fd, (struct sockaddr*) &client_address, &client_len);
  // char recv_buf[1024];
  uint8_t recv_buf;
  char send_buf[2048];

  while(1)
  {
    memset(&recv_buf, 0, sizeof(recv_buf));
    // recv(client_fd, recv_buf, 1024, 0);
    // recv_buf[1023] = '\0';
    recv(client_fd, &recv_buf, sizeof(recv_buf), 0);
    
    printf("Received message %x\n", recv_buf);

    memset(send_buf, 0, 2048);
    snprintf(send_buf, 2048, "Reply from server: Received message %x\n", recv_buf);
    send(client_fd, send_buf, 2048, 0);
    sleep(1);

  }
  close(client_fd);
  close(listen_fd);

  return 0;
}


