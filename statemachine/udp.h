#ifndef UDP_H
#define UDP_H

#include <pthread.h>

// Function prototypes
void* udp_tlm_thread_fn(void* arg);
void* udp_cmd_thread_fn(void* arg);

#endif

