#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "statemachine.h"
#include "udp.h" 
void* udpcomms_thread(void* arg);


int main() {
    
    pthread_t statemachine_tid, udp_tlm_tid, udp_cmd_tid;
    init_udp_sockets();
    // Create threads for statemachine and udp.
    pthread_create(&statemachine_tid, NULL, statemachine_thread, NULL);
    pthread_create(&udp_tlm_tid, NULL, udp_tlm_thread_fn, NULL);
    pthread_create(&udp_cmd_tid, NULL, udp_cmd_thread_fn, NULL);

    
    pthread_join(statemachine_tid, NULL);
    pthread_join(udp_tlm_tid, NULL); 
    pthread_join(udp_cmd_tid, NULL);
    return 0;
}