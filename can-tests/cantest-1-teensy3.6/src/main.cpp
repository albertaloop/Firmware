#include <FlexCAN.h>
#include <Arduino.h>

#define ISR_INTERVAL 1000000 // 100,000 microseconds = 50 ms
#define total_responses 255

FlexCAN fc = FlexCAN(500000);



#if 0
IntervalTimer interruptTimer;
// this keeps track of whether we're using the interrupt
// off by default!
boolean usingInterrupt = false;
void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy
volatile bool interrupt_flag = false;
enum { state0 = 0, state1, state2, state3 };
int state = 0;

void timerCallback(void) { interrupt_flag = true; }

#endif

CAN_message_t send_msg;
CAN_message_t recv_msg;
CAN_filter_t mask = {(uint8_t)0x00, (uint8_t)0x00, (uint32_t)0x04fffff};

typedef union {
  float number;
  uint8_t bytes[4];
} FLOATUNION_t;

typedef union {
  char string[8];
  uint8_t bytes[8];
} STRINGUNION_t;

void (*responses[total_responses])(CAN_message_t msg_in);

void canTest2Callback(CAN_message_t msg_in) {
  // for (int i = 0; i < 3; i++) {
  //   est.x[i] = 0;
  // }
  Serial.println("canTest2Callback");
  Serial.print("Received message with ID:");

}

// Define a default callback function
void defaultCallback(CAN_message_t msg_in) {
  Serial.print("Unhandled CAN message with ID: ");
  Serial.println(msg_in.id);
  Serial.print("Received data: ");
  STRINGUNION_t msg;
  memcpy(msg.bytes, msg_in.buf, 8);
  Serial.println(msg.string);
}



void canbus_loop(void) {
  Serial.println("CANbus loop");

  // receive message
  CAN_message_t msg_in;
  msg_in.timeout = 1;
  if (fc.read(msg_in)) {
    // respond to message
    Serial.print("CAN test 1 received message: ");
    Serial.println(msg_in.id);
    uint8_t msg_id = (msg_in.id >> 24);
    Serial.println(msg_id);
    responses[msg_in.id >> 24](msg_in);
  }
  // Send telemetry
  STRINGUNION_t str_data;
  const char * msg1 = "msg1";
  memcpy(str_data.string, msg1, 5);
  for (int i = 0; i < 5; i++) {
    send_msg.buf[i] = str_data.bytes[i];
  }
  fc.write(send_msg);

}

void setup() {

  // Initialize all elements of the responses array to the default callback
  for (int i = 0; i < total_responses; ++i) {
    responses[i] = defaultCallback;
  }

  responses[4] = canTest2Callback;
  //responses[15] = recenterCallback;

  send_msg.ext = 0;
  send_msg.id = 0x01ffffff;
  send_msg.len = 8;
  memset(send_msg.buf, 0, 8);

  // recv_msg.ext = 0;
  // recv_msg.id = 0x03ffffff;
  // recv_msg.len = 4;
  // memset(recv_msg.buf, 0, 8);


  Serial.begin(9600);
  Serial.println("CANbus basic test!");


  // interruptTimer.begin(timerCallback, ISR_INTERVAL);
  //fc.begin(mask);
  fc.begin();

}

void loop() {
  canbus_loop();
  delay(1000);
}
