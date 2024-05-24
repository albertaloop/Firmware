#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>


int main()
{
  int s;
  struct sockaddr_can addr;
  struct ifreq ifr;
  struct can_frame frame;

	if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("Socket");
		return 1;
	}

  strcpy(ifr.ifr_name, "vcan0" );
  ioctl(s, SIOCGIFINDEX, &ifr);

  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Bind");
		return 1;
	}

  frame.can_id = 0x555;
	frame.can_dlc = 5;
	sprintf(frame.data, "Hello");


	if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
		perror("Write");
		return 1;
	}



  nbytes = read(s, &frame, sizeof(struct can_frame));

  if (nbytes < 0) {
          perror("can raw socket read");
          return 1;
  }

  /* paranoid check ... */
  if (nbytes < sizeof(struct can_frame)) {
          fprintf(stderr, "read: incomplete CAN frame\n");
          return 1;
  }

  /* do something with the received CAN frame */

  nbytes = write(s, &frame, sizeof(struct can_frame));

	if (close(s) < 0) {
		perror("Close");
		return 1;
	}

  return 0;
}