#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


int sock_fd;

struct addrinfo *pod_address;
struct addrinfo hints;
//hints.ai_family = AF_INET;


void setup_sockets()
{
  hints.ai_socktype = SOCK_STREAM;

  int rv;
  rv = getaddrinfo("192.168.1.10", "5000", &hints, &pod_address);
  // rv = getaddrinfo("127.0.0.1", "5000", &hints, &pod_address);

  if (rv != 0)
  {
    perror("Could not get requested address.");
    exit(1);
  }

  sock_fd = socket(pod_address->ai_family , pod_address->ai_socktype, pod_address->ai_protocol);
  if (sock_fd < 0)
  {
    perror("Could not create socket.");
    exit(1);
  }

  freeaddrinfo(pod_address);

  rv = connect(sock_fd, pod_address->ai_addr, pod_address->ai_addrlen);
  if (rv < 0)
  {
    perror("Could not connect to pod.");
    exit(1);
  }

}

int main()
{
  setup_sockets();

  char recv_buf[2048];
  char send_buf[1024];

  while(1)
  {
    memset(send_buf, 0, 1024);
    snprintf(send_buf, 1024, "Test command.");
    send(sock_fd, send_buf, 1024, 0);

    memset(recv_buf, 0, 2048);
    recv(sock_fd, recv_buf, 2048, 0);
    recv_buf[2047] = '\0';
    printf("Received message %s\n", recv_buf);

    sleep(1);

  }
  close(sock_fd);

  return 0;
}


