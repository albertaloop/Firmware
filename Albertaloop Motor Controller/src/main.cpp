#include <Arduino.h>
#include <ChRt.h>

#include "circular_buffer.h"

#define total_responses 255

#define USART1 Serial1


FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;

CircularBuffer<int, 10> usart_recv_cbuf;

#define NUM_TX_MAILBOXES 2
#define NUM_RX_MAILBOXES 6


uint8_t status;
int32_t x[3] = {1, 1, 1};
// CircularBuffer<CAN_message_t, 10> output_buffer;

unsigned long wait_time = 40; // 40 ms

uint8_t team_id = 0;

void (*responses[255])( void );

THD_WORKING_AREA(waThread1, 128);

// CANbus communications
static THD_FUNCTION(Thread1, arg) {
  (void)arg;

  delay(1);
  chThdYield();
}

THD_WORKING_AREA(waThread2, 128);
static THD_FUNCTION(Thread2, arg) {
  (void)arg;

  delay(1);
  chThdYield();
}

THD_WORKING_AREA(waThread3, 128);

// State machine
static THD_FUNCTION(Thread3, arg) {
  (void)arg;

  delay(1);
  chThdYield();
}

void chSetup() {
  chThdCreateStatic(waThread1, sizeof(waThread1),
    NORMALPRIO, Thread1, NULL);
  chThdCreateStatic(waThread2, sizeof(waThread2),
    NORMALPRIO, Thread2, NULL);
  chThdCreateStatic(waThread3, sizeof(waThread3),
    NORMALPRIO, Thread3, NULL);
}

void setup() {
  chSysInit();
  chBegin(chSetup);

  Serial.begin(9600);
  delay(400);
  can1.begin();
  can1.setBaudRate(1000000);
  can1.setMaxMB(NUM_TX_MAILBOXES + NUM_RX_MAILBOXES);
    for (int i = 0; i<NUM_RX_MAILBOXES; i++){
    can1.setMB((FLEXCAN_MAILBOX)i,RX,EXT);
  }
  for (int i = NUM_RX_MAILBOXES; i<(NUM_TX_MAILBOXES + NUM_RX_MAILBOXES); i++){
    can1.setMB((FLEXCAN_MAILBOX)i,TX,EXT);
  }
  can1.setMBFilter(REJECT_ALL);

}

void loop() {
  Serial.println("Main thread");

}     