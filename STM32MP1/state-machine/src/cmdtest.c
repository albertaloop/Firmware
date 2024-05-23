#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

// Define the command constants
#define FORWARD 0xc4
#define STOP 0xc0
#define REVERSE_ON 0xc6
#define REVERSE_OFF 0xc8
#define INVALID 0xff


int reverse_state = REVERSE_OFF;

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

uint8_t get_next_cmd()
{
  printf("Enter the next Command:\n" \
  "w : Forward\n" \
  "s : Stop\n" \
  "r : Toggle Reverse\n");

  char text_input;
  // int rv;
  // while ((rv = scanf(" %c", &text_input)) != 1) {
  //   // Skip invalid input
  //   if (rv == EOF) {
  //     perror("Error reading input");
  //     return INVALID;
  //   }
  //   while (getchar() != '\n'); // Clear invalid input
  // }

  text_input = getchar();
  getchar(); // consume new line character

  printf("%c", text_input);
  switch (text_input)
  {
    case 'w':
      printf("Entered:\n-----FORWARD-----\n");
      return FORWARD;
    case 's':
      printf("Entered:\n-----STOP-----\n");
      return STOP;
    case 'r':
      if (reverse_state == REVERSE_ON)
      {
        printf("Entered:\n-----REVERSE OFF-----\n");
        reverse_state = REVERSE_OFF;
        return REVERSE_OFF;
      }
      else
      {
        printf("Entered:\n-----REVERSE ON-----\n");
        reverse_state = REVERSE_ON;
        return REVERSE_ON;
      }
    default:
      printf("Invalid command received.\n");
      return INVALID;
  }


}

int main()
{
  setup_sockets();

  char recv_buf[2048];
  char send_buf[1024];
  

  while(1)
  {

    uint8_t cmd_msg = get_next_cmd();
    if (cmd_msg != INVALID)
    {
      if (send(sock_fd, &cmd_msg, sizeof(cmd_msg), 0) < 0)
      {
        perror("Send failed");
        close(sock_fd);
        return 1;
      }
    }

    memset(recv_buf, 0, 2048);
    recv_buf[2047] = '\0';
    if (recv(sock_fd, recv_buf, 2048, 0) < 0)
    {
      perror("Receive failed");
    }
    printf("Received reply from pod:\n %s\n", recv_buf);

  }
  close(sock_fd);

  return 0;
}


