#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <errno.h>
#include <linux/can.h>
#include <linux/can/raw.h>

int can_sock;
struct sockaddr_can addr;
struct ifreq ifr;
struct can_frame send_frame;


int loop_count = 0;
struct timeval tv;


static void can_task()
{
    printf("CANbus task %d\n", loop_count++);

    ssize_t nbytes;

    printf("Writing socket\n");


	nbytes = write(can_sock, &send_frame, sizeof(struct can_frame));
    if (nbytes == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            printf("The write operation would block, try again later\n");
        }
        else
        {
            perror("Write");
            exit(1);
        }
    }
    else if (nbytes != sizeof(struct can_frame))
    {
        perror("write: incomplete CAN frame\n");

    }
    else
    {
        printf("Successfully wrote %zd bytes\n", nbytes);
	}

    printf("Reading socket\n");

    struct can_frame recv_frame;
    memset(&recv_frame, 0, sizeof(struct can_frame));
    nbytes = read(can_sock, &recv_frame, sizeof(struct can_frame));
    printf("%zd bytes received", nbytes);
    if (nbytes == -1)
    {
        printf("hello??");
        perror("can raw socket read");
        //exit(1);
    }
    else if (nbytes == 0)
    {
        printf("No CAN message received");
    }
    else if (nbytes < sizeof(struct can_frame)) {
        fprintf(stderr, "read: incomplete CAN frame\n");
        exit(1);
    }
    else {
        char data[9];
        memcpy(data, recv_frame.data, 8);
        data[8] = '\0';
        for (int i = 0; i < 8; ++i)
        {
            printf("%c", recv_frame.data[i]);
        }
        printf("\n");
        for (int i = 0; i < 8; ++i)
        {
            printf("%d", recv_frame.data[i]);
        }
        printf("Received message \"%s\" with id %d\n", data, recv_frame.can_id);
    }  




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
    printf("Initializing CAN socket\n");
	if ((can_sock = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("Socket");
		return 1;
	}
    strcpy(ifr.ifr_name, "can0" );
    if (ioctl(can_sock, SIOCGIFINDEX, &ifr) == -1) {
        perror("ioctl SIOCGIFINDEX");
        close(can_sock);
        return 1;
    }


    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    send_frame.can_id = 0x4ff;
	send_frame.can_dlc = 5;
	sprintf(send_frame.data, "msg2");

	if (bind(can_sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		perror("Bind");
		return 1;
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
    if (rv == -1) {
        perror("setsockopt filter");
    }


    // Enable reception of CAN FD frames
    int enable = 1;
    rv = setsockopt(can_sock, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable, sizeof(int));
    if (rv == -1)
    {
        perror("setsockopt CAN FD");
    }
    // 100ms delay
    struct timespec loop_delay;
    loop_delay.tv_sec = 1;
    loop_delay.tv_nsec = 0;
    while(1)
    {
        can_task();
        nanosleep(&loop_delay, NULL);
    }


    if (close(can_sock) < 0) {
        perror("Close");
        return 1;
    }

  return 0;
}