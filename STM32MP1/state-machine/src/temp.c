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
#define WATCHDOG          0x99

struct prio_queue p_queue;

struct prio_queue response_queue;


bool soundOffFlag = true;
volatile sig_atomic_t timer_expired = 0;

static void (*tcp_responses[MSG_ID_SPACE])(uint8_t msg_in);

int watchdog_count = 100000;

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

static void watchDogCallback(uint8_t msg_in) {
  watchdog_count = 100000;
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
  // Prevents the error when we try to restart the program, but it says the port is in use
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

  /* Setup CAN connection */
  setup_can();

  // Configure loop delay
  struct timespec loop_delay;
  loop_delay.tv_sec = 0;
  loop_delay.tv_nsec = 1000000;

  /* Begin CAN communication */
  while(1)
  {

    // printf("CAN: CANbus task %d\n", loop_count++);

    ssize_t nbytes;
    msg cmd_msg;

    // Get message from priority queue
    if(pop_msg(&p_queue, &cmd_msg))
    {
      printf("Writing socket\n");
      // Prepare outgoing CAN message
      memset(send_frame.data, 0, 8);
      send_frame.can_id = cmd_msg.id;
	    send_frame.can_dlc = 1;
      send_frame.data[0] = cmd_msg.val;

      // Broadcast CAN message
      nbytes = write(can_sock, &send_frame, sizeof(struct can_frame));
      if (nbytes == -1)
      {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
          // Couldn't send message. Would block if
          // not using non-blocking fd.
          printf("CAN: The write operation would block, try again later\n");
        }
        else
        {
          // Write error
          perror("can raw socket write");
          exit(1);
        }
      }
      else if (nbytes != sizeof(struct can_frame))
      {
        // Did not write a complete CAN frame
        perror("CAN: write: incomplete CAN frame\n");
      }
      else
      {
        // Write successful
        printf("CAN: Successfully wrote %zd bytes\n", nbytes);
      }

#if 1
      printf("CAN: Reading socket\n");

      // Prepare to receive CAN message
      struct can_frame recv_frame;
      memset(&recv_frame, 0, sizeof(struct can_frame));

      // Read incoming CAN message
      nbytes = read(can_sock, &recv_frame, sizeof(struct can_frame));
      printf("%zd bytes received", nbytes);
      if (nbytes == -1)
      {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
          // No message to read. Would block if not
          // using non-blocking fd
          printf("CAN: The write operation would block, try again later\n");
        }
        else
        {
          // Read error
          perror("can raw socket read");
          exit(1);
        }

      }
      else if (nbytes != sizeof(struct can_frame))
      {
        // Did not read a complete CAN frame
        fprintf(stderr, "read: incomplete CAN frame\n");
        exit(1);
      }
      else
      {
        // Read successful

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
        // TO-DO - Invoke CAN resposne callback here

      }
#endif
      
    }
#if 0
    if (timer_expired)
    {
      // Soundoff timer expired
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
      printf("Sending brake soundoff\n");
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
#endif
    
    nanosleep(&loop_delay, NULL);
  }
}

static void * tcp_task(void *arg)
{
  /* Setup TCP callback functions */

  // Initialize all elements of the responses array to the default callback
  for (int i = 0; i < MSG_ID_SPACE; ++i) {
    tcp_responses[i] = defaultTCPCallback;
  }

  // Motor command responses
  tcp_responses[MOTOR_FORWARD] = cmdForwardTCPCallback;
  tcp_responses[MOTOR_STOP] = cmdStopTCPCallback;
  tcp_responses[MOTOR_REVERSE_OFF] = cmdReverseOffTCPCallback;
  tcp_responses[MOTOR_REVERSE_ON] = cmdReverseOnTCPCallback;

  // Brake command responses
  tcp_responses[BRAKE_LAUNCH] = cmdBrakeLaunchTCPCallback;
  tcp_responses[BRAKE_ESTOP] = cmdBrakeStopTCPCallback;
  tcp_responses[BRAKE_CRAWL] = cmdBrakeCrawlTCPCallback;
  tcp_responses[BRAKE_PREP_LAUNCH] = cmdBrakePrepLaunchTCPCallback;

  // LED command responses
  tcp_responses[LED_FAULT] = cmdLEDFaultTCPCallback;
  tcp_responses[LED_NORMAL] = cmdLEDNormalTCPCallback;

  /* Setup TCP connection */
  setup_sockets();
  struct sockaddr_storage client_address;
  socklen_t client_len = sizeof(client_address);

  // Wait for incoming connection
  int client_fd = accept(listen_fd, (struct sockaddr*) &client_address, &client_len);
  set_nonblocking(client_fd);

  // Allocate message buffers
  uint8_t recv_buf;
  char send_buf[2048];

  // Configure loop delay
  struct timespec loop_delay;
  loop_delay.tv_sec = 0;
  loop_delay.tv_nsec = 1000000;
  int nbytes;

  // Begin TCP communication
  while(1)
  {
    #if 0 // Not receiving watchdog resest message for now
    watchdog_count--;
    if (watchdog_count <= 0)
    {
      // Disable watchdog timer when it expires
      printf("watchdog went off");
      disable_timer();
      printf("Timer disabled\n");
    }
    #endif
    // Clear contents of receive buffer
    memset(&recv_buf, 0, sizeof(recv_buf));

    // Read incoming commands
    nbytes = recv(client_fd, &recv_buf, sizeof(recv_buf), 0);
  

    if (nbytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data was available, so it would block if
            // we weren't using non-blocking fd
            // This was spamming too many print messages
            //printf("Receive would block, try again later\n");
        } else {
            // Recv failed
            perror("Recv");
            disable_timer();
            printf("Timer disabled\n");
        }
    } else if (nbytes == 0) {
        // Connection closed normally
        printf("Connection closed by the server\n");
        disable_timer();
        printf("Timer disabled\n");
    } else {
      printf("TCP: Received message %x\n", recv_buf);

      // Send reply to server to confirm command was received.
      memset(send_buf, 0, 2048);
      snprintf(send_buf, 2048, "Reply from server: Received message %x\n", recv_buf);
      send(client_fd, send_buf, 2048, 0);
      // Call the response callback function
      tcp_responses[recv_buf](recv_buf);
    }

    nanosleep(&loop_delay, NULL);
  }
  close(client_fd);
}


/*
* - Set file descriptor to be non-blocking
* - This is requried for send and recv
* to not block.
*/
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

  /* Create soundoff timer with interval timer*/

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

#if 0
  // Start the timer
  setitimer(ITIMER_REAL, &timer, NULL);
#endif

  /* Create Priority Queues*/
  init_pqueue(&p_queue, QUEUE_SIZE, QUEUE_SIZE, QUEUE_SIZE);
  init_pqueue(&response_queue, QUEUE_SIZE, QUEUE_SIZE, QUEUE_SIZE);

  /* Create CAN thread */
  pthread_create(&can_thread, NULL, can_task, NULL);
  // Create TCP thread
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
