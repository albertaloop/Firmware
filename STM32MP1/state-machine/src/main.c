#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <signal.h>
#include <netdb.h>
#include <sys/types.h>
#include <errno.h>
#include <linux/can.h>
#include <linux/can/raw.h>


#include <pthread.h>


#include "priority_queue.h"

#define CAN_STATE_M_BRAKE_ID 0x3ff
#define CAN_STATE_M_MOTOR_ID 0x4ff
#define CAN_STATE_M_LED_ID   0x5ff

#define MSG_ID_SPACE 255
#define MOTOR_FORWARD     0xc4
#define MOTOR_STOP        0xc0
#define MOTOR_REVERSE_ON  0xc6
#define MOTOR_REVERSE_OFF 0xc8

#define BRAKE_ESTOP       0xa0
#define BRAKE_PREP_LAUNCH 0xa2
#define BRAKE_LAUNCH      0xa4
#define BRAKE_CRAWL       0xa6

#define LED_FAULT         0xb1
#define LED_NORMAL        0xb0
#define SOUND_OFF         0x33

struct prio_queue p_queue;

struct prio_queue response_queue;


bool soundOffFlag = true;
volatile sig_atomic_t timer_expired = 0;

static void (*tcp_responses[MSG_ID_SPACE])(uint8_t msg_in);



void (*can_responses[MSG_ID_SPACE])();


int can_sock;
struct sockaddr_can addr;
struct ifreq ifr;
struct can_frame send_frame;

int listen_fd;

struct addrinfo *bind_address;
struct addrinfo hints;

pthread_t can_thread;
pthread_t tcp_thread;

// int loop_count = 0;
struct timeval tv;

static void timer_handler(int signum) {
    timer_expired = 1;
}

static void disable_timer() {
    struct itimerval timer;

    // Set the timer to zero to disable it
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    // Stop the timer
    setitimer(ITIMER_REAL, &timer, NULL);
}

// Define a default callback function
static void defaultTCPCallback(uint8_t msg_in) {
  printf("TCP: Unhandled tcp message : %x\n", msg_in);
}

/* Motor command callbacks */

static void cmdForwardTCPCallback(uint8_t msg_in) {
  printf("TCP: Received forward message : %x\n", msg_in);
  msg cmd_msg = {PRIO_MED, msg_in, CAN_STATE_M_MOTOR_ID};
  push_msg(&p_queue, cmd_msg);
}

static void cmdStopTCPCallback(uint8_t msg_in) {
  printf("TCP: Received stop message : %x\n", msg_in);
  msg cmd_msg = {PRIO_HIGH, msg_in, CAN_STATE_M_MOTOR_ID};
  push_msg(&p_queue, cmd_msg);
}

static void cmdReverseOnTCPCallback(uint8_t msg_in) {
  printf("TCP: Received reverse ON message : %x\n", msg_in);
  msg cmd_msg = {PRIO_MED, msg_in, CAN_STATE_M_MOTOR_ID};
  push_msg(&p_queue, cmd_msg);
}

static void cmdReverseOffTCPCallback(uint8_t msg_in) {
  printf("TCP: Received reverse OFF message : %x\n", msg_in);
  msg cmd_msg = {PRIO_MED, msg_in, CAN_STATE_M_MOTOR_ID};
  push_msg(&p_queue, cmd_msg);
}

/* Brake command callbacks */

static void cmdBrakeCrawlTCPCallback(uint8_t msg_in) {
  printf("TCP: Received Brake Engage message : %x\n", msg_in);
  msg cmd_msg = {PRIO_MED, msg_in, CAN_STATE_M_BRAKE_ID};
  push_msg(&p_queue, cmd_msg);
}

static void cmdBrakeLaunchTCPCallback(uint8_t msg_in) {
  printf("TCP: Received Brake Release : %x\n", msg_in);
  msg cmd_msg = {PRIO_HIGH, msg_in, CAN_STATE_M_BRAKE_ID};
  push_msg(&p_queue, cmd_msg);
}

static void cmdBrakePrepLaunchTCPCallback(uint8_t msg_in) {
  printf("TCP: Received Prepare Launch message : %x\n", msg_in);
  msg cmd_msg = {PRIO_MED, msg_in, CAN_STATE_M_BRAKE_ID};
  push_msg(&p_queue, cmd_msg);
}

static void cmdBrakeStopTCPCallback(uint8_t msg_in) {
  printf("TCP: Received Brake Power Down message : %x\n", msg_in);
  msg cmd_msg = {PRIO_MED, msg_in, CAN_STATE_M_BRAKE_ID};
  push_msg(&p_queue, cmd_msg);
}

/* LED command callbacks */

static void cmdLEDNormalTCPCallback(uint8_t msg_in) {
  printf("TCP: Received reverse ON message : %x\n", msg_in);
  msg cmd_msg = {PRIO_LOW, msg_in, CAN_STATE_M_LED_ID};
  push_msg(&p_queue, cmd_msg);
}

static void cmdLEDFaultTCPCallback(uint8_t msg_in) {
  printf("TCP: Received Brake Stop message : %x\n", msg_in);
  msg cmd_msg = {PRIO_LOW, msg_in, CAN_STATE_M_LED_ID};
  push_msg(&p_queue, cmd_msg);
}

static void setup_can()
{
  printf("Initializing CAN socket\n");
	if ((can_sock = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
  {
		perror("Socket");
		exit(1);
	}
  strcpy(ifr.ifr_name, "can0" );
  if (ioctl(can_sock, SIOCGIFINDEX, &ifr) == -1)
  {
      perror("ioctl SIOCGIFINDEX");
      close(can_sock);
      exit(1);
  }


  memset(&addr, 0, sizeof(addr));
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;

  send_frame.can_id = 0x4ff;
	send_frame.can_dlc = 1;
  memset(send_frame.data, 0, 8);
	// sprintf(send_frame.data, "msg2");

	if (bind(can_sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
  {
		perror("Bind");
		exit(1);
	}

  //set_nonblocking(can_sock);


  struct can_filter filter[3];
  filter[0].can_id   = 0x3ff;
  filter[0].can_mask = CAN_SFF_MASK;
  filter[1].can_id   = 0x4ff;
  filter[1].can_mask = CAN_SFF_MASK;
  filter[2].can_id   = 0x320;
  filter[2].can_mask = CAN_SFF_MASK;

  int rv = setsockopt(
      can_sock,
      SOL_CAN_RAW,
      CAN_RAW_FILTER,
      &filter,
      sizeof(filter)
  );
  if (rv == -1)
  {
      perror("setsockopt filter");
  }


  // Enable reception of CAN FD frames
  int enable = 1;
  rv = setsockopt(can_sock, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable, sizeof(int));
  if (rv == -1)
  {
    perror("setsockopt CAN FD");
  }
}



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


static void * can_task(void *arg)
{
  setup_can();

  // // 1s delay
  struct timespec loop_delay;
  loop_delay.tv_sec = 0;
  loop_delay.tv_nsec = 1000;


  while(1)
  {

    // printf("CAN: CANbus task %d\n", loop_count++);

    ssize_t nbytes;
    msg cmd_msg;
    if(pop_msg(&p_queue, &cmd_msg))
    {
      printf("Writing socket\n");
      memset(send_frame.data, 0, 8);
      send_frame.can_id = cmd_msg.id;
	    send_frame.can_dlc = 1;
      send_frame.data[0] = cmd_msg.val;
      nbytes = write(can_sock, &send_frame, sizeof(struct can_frame));
      if (nbytes == -1)
      {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
          printf("CAN: The write operation would block, try again later\n");
        }
        else
        {
          perror("can raw socket write");
          exit(1);
        }
      }
      else if (nbytes != sizeof(struct can_frame))
      {
        perror("CAN: write: incomplete CAN frame\n");
      }
      else
      {
        printf("CAN: Successfully wrote %zd bytes\n", nbytes);
      }

#if 1
    }
      printf("CAN: Reading socket\n");

      struct can_frame recv_frame;
      memset(&recv_frame, 0, sizeof(struct can_frame));
      nbytes = read(can_sock, &recv_frame, sizeof(struct can_frame));
      printf("%zd bytes received", nbytes);
      if (nbytes == -1)
      {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
          printf("CAN: The write operation would block, try again later\n");
        }
        else
        {
          perror("can raw socket read");
          exit(1);
        }

      }
      else if (nbytes != sizeof(struct can_frame))
      {
        fprintf(stderr, "read: incomplete CAN frame\n");
        exit(1);
      }
      else
      {
        // char data[9];
        // memcpy(data, recv_frame.data, 8);
        // data[8] = '\0';
        // for (int i = 0; i < 8; ++i)
        // {
        //   printf("%c", recv_frame.data[i]);
        // }
        // printf("\n");
        // for (int i = 0; i < 8; ++i)
        // {
        //   printf("%d", recv_frame.data[i]);
        // }
        printf("CAN: Received message \"%x\" with id %d\n", recv_frame.data[0], recv_frame.can_id);
        push_msg(&response_queue, recv_frame.data[0]);
      }
      
#else 
    }
#endif

    if (timer_expired)
    {
      timer_expired = 0;
      // Send motor sound off
      printf("Sending motor soundoff\n");
      memset(send_frame.data, 0, 8);
      send_frame.can_id = CAN_STATE_M_MOTOR_ID;
	    send_frame.can_dlc = 1;
      send_frame.data[0] = SOUND_OFF;
      nbytes = write(can_sock, &send_frame, sizeof(struct can_frame));
      if (nbytes == -1)
      {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
          printf("CAN: The write operation would block, try again later\n");
        }
        else
        {
          perror("can raw socket write");
          exit(1);
        }
      }
      else if (nbytes != sizeof(struct can_frame))
      {
        perror("CAN: write: incomplete CAN frame\n");
      }
      else
      {
        printf("CAN: Successfully wrote %zd bytes\n", nbytes);
      }

      // Send brake sound off
      printf("Sending brake  soundoff\n");
      memset(send_frame.data, 0, 8);
      send_frame.can_id = CAN_STATE_M_BRAKE_ID;
	    send_frame.can_dlc = 1;
      send_frame.data[0] = SOUND_OFF;
      nbytes = write(can_sock, &send_frame, sizeof(struct can_frame));
      if (nbytes == -1)
      {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
          printf("CAN: The write operation would block, try again later\n");
        }
        else
        {
          perror("can raw socket write");
          exit(1);
        }
      }
      else if (nbytes != sizeof(struct can_frame))
      {
        perror("CAN: write: incomplete CAN frame\n");
      }
      else
      {
        printf("CAN: Successfully wrote %zd bytes\n", nbytes);
      }
    }

    
    nanosleep(&loop_delay, NULL);
  }
}


static void * tcp_task(void *arg)
{

  // Initialize all elements of the responses array to the default callback
  for (int i = 0; i < MSG_ID_SPACE; ++i) {
    tcp_responses[i] = defaultTCPCallback;
  }

  tcp_responses[MOTOR_FORWARD] = cmdForwardTCPCallback;
  tcp_responses[MOTOR_STOP] = cmdStopTCPCallback;
  tcp_responses[MOTOR_REVERSE_OFF] = cmdReverseOffTCPCallback;
  tcp_responses[MOTOR_REVERSE_ON] = cmdReverseOnTCPCallback;

  tcp_responses[BRAKE_LAUNCH] = cmdBrakeLaunchTCPCallback;
  tcp_responses[BRAKE_ESTOP] = cmdBrakeStopTCPCallback;
  tcp_responses[BRAKE_CRAWL] = cmdBrakeCrawlTCPCallback;
  tcp_responses[BRAKE_PREP_LAUNCH] = cmdBrakePrepLaunchTCPCallback;

  tcp_responses[LED_FAULT] = cmdLEDFaultTCPCallback;
  tcp_responses[LED_NORMAL] = cmdLEDNormalTCPCallback;

  setup_sockets();
  struct sockaddr_storage client_address;
  socklen_t client_len = sizeof(client_address);
  // Wait for connection
  int client_fd = accept(listen_fd, (struct sockaddr*) &client_address, &client_len);

  set_nonblocking(client_fd);
  // char recv_buf[1024];
  uint8_t recv_buf;
  char send_buf[2048];
  // // 1s delay
  struct timespec loop_delay;
  loop_delay.tv_sec = 0;
  loop_delay.tv_nsec = 1000000;
  int nbytes;
  uint8_t send_msg;
  while(1)
  {
    memset(&recv_buf, 0, sizeof(recv_buf));
    // recv(client_fd, recv_buf, 1024, 0);
    // recv_buf[1023] = '\0';
    nbytes = recv(client_fd, &recv_buf, sizeof(recv_buf), 0);
    
    if (nbytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            //printf("Receive would block, try again later\n");
      } else {
        disable_timer();
        printf("Timer disabled\n");
      }
    } else if (nbytes == 0) {
        printf("Connection closed by the server\n");
        disable_timer();
        printf("Timer disabled\n");
    } else {
      printf("TCP: Received message %x\n", recv_buf);

      memset(send_buf, 0, 2048);
      snprintf(send_buf, 2048, "Reply from server: Received message %x\n", recv_buf);
      send(client_fd, send_buf, 2048, 0);

      tcp_responses[recv_buf](recv_buf);
    }

    if(pop_msg(&response_queue, &send_msg))
    {
      send(client_fd, &send_msg, sizeof(send_msg), 0);
    }

    nanosleep(&loop_delay, NULL);
  }
  close(client_fd);
}

void set_nonblocking(int fd)
{
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0)
  {
      perror("Could not get socket flags with F_GETFD");
      exit(1);
  }

  flags |= O_NONBLOCK;
  int rv = fcntl(fd, F_SETFL, flags);
  if (rv < 0)
  {
      perror("Could not set socket flags with F_SETFD");
      exit(1);
  }

  fcntl(fd, O_NONBLOCK);
}


int main()
{

    struct sigaction sa;
    struct itimerval timer;

    // Install timer_handler as the signal handler for SIGALRM.
    sa.sa_handler = &timer_handler;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGALRM, &sa, NULL);

    // Configure the timer to expire after 20 ms...
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 20000;
    // ... and every 20 ms after that.
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 20000;

    // Start the timer
    setitimer(ITIMER_REAL, &timer, NULL);

  init_pqueue(&p_queue, QUEUE_SIZE, QUEUE_SIZE, QUEUE_SIZE);
  init_pqueue(&response_queue, QUEUE_SIZE, QUEUE_SIZE, QUEUE_SIZE);


  pthread_create(&can_thread, NULL, can_task, NULL);
  pthread_create(&tcp_thread, NULL, tcp_task, NULL);


  pthread_join(can_thread, NULL);
  pthread_join(tcp_thread, NULL); 

  if (close(can_sock) < 0)
  {
    perror("CANbus close");
    return 1;
  }

  if (close(listen_fd) < 0)
  {
    perror("TCP close");
    return 1;
  }

  return 0;
}