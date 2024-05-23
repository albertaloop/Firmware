#ifndef _PRIO_Q_
#define _PRIO_Q_

#include <stdbool.h>
#include <stddef.h>

#define NUM_PRIO 3
#define QUEUE_SIZE 10
#define PRIO_HIGH 0
#define PRIO_MED 1
#define PRIO_LOW 2

typedef struct msg {
    int prio;
    int val;
} msg;

typedef struct queue {
    struct msg msgs[QUEUE_SIZE];
    int count;
    int size;
} queue;

typedef struct prio_queue {
    struct queue queues[NUM_PRIO];
} prio_queue;

void init_pqueue(struct prio_queue *pq,
                 size_t high_prio_size,
                 size_t med_prio_size,
                 size_t low_prio_size);

bool is_empty(struct queue *q);

bool is_full(struct queue *q);

bool push_msg(struct prio_queue *pq, struct msg m);

bool pop_msg(struct prio_queue *pq, struct msg *m);

bool check_prempt(struct prio_queue *pq, struct msg m);

bool insert_front(struct prio_queue *pq, struct msg m);

int get_count(struct prio_queue *pq);

void print_queue(struct prio_queue *pq);

#endif