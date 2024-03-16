#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "statemachine.h"

void* udpcomms_thread(void* arg);


int main() {
    
    pthread_t statemachine_tid, udpcomms_tid;

    // Create threads for statemachine and udp.
    pthread_create(&statemachine_tid, NULL, statemachine_thread, NULL);
    // pthread_create(&udpcomms_tid, NULL, udpcomms_thread, NULL);

    
    pthread_join(statemachine_tid, NULL);
    // pthread_join(udpcomms_tid, NULL);

    return 0;
}