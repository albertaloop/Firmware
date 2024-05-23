#include "priority_queue.h"
#include <stdbool.h>
#include <stdio.h>

static bool queue_sl(struct queue *q);

static bool queue_sr(struct queue *q);

void init_pqueue(struct prio_queue *pq,
                 size_t high_prio_size,
                 size_t med_prio_size,
                 size_t low_prio_size) {
    pq->queues[PRIO_HIGH].count = 0;
    pq->queues[PRIO_HIGH].size = high_prio_size;
    pq->queues[PRIO_MED].count = 0;
    pq->queues[PRIO_MED].size = med_prio_size;
    pq->queues[PRIO_LOW].count = 0;
    pq->queues[PRIO_LOW].size = low_prio_size;
}

bool is_empty(struct queue *q) {
    if(q->count == 0) return true;
    else return false;
}

bool is_full(struct queue *q) {
    if(q->count >= q->size) {
        return true;
    } 
    return false;
}

bool push_msg(struct prio_queue *pq, struct msg msg_in) {
    if(msg_in.prio != PRIO_HIGH &&
       msg_in.prio != PRIO_MED &&
       msg_in.prio != PRIO_LOW) {
        printf("invalid priority\n");
        return false;
    }
    struct queue * q = &pq->queues[msg_in.prio];
    if(!is_full(q)) {
        const int count = q->count;
        q->msgs[count] = msg_in;
        q->count++;
        return true;
    } else {
        printf("queue full\n");
        return false;
    }
    
}

bool pop_msg(struct prio_queue *pq, struct msg *msg_in) {
    if(!is_empty(&(pq->queues[PRIO_HIGH]))) {
        *msg_in = pq->queues[PRIO_HIGH].msgs[0];
        return queue_sl(&(pq->queues[PRIO_HIGH]));
    } else if(!is_empty(&(pq->queues[PRIO_MED]))) {
        *msg_in = pq->queues[PRIO_MED].msgs[0];
        return queue_sl(&(pq->queues[PRIO_MED]));
    } else if(!is_empty(&(pq->queues[PRIO_LOW]))) {
        *msg_in = pq->queues[PRIO_LOW].msgs[0];
        return queue_sl(&(pq->queues[PRIO_LOW]));
    }
    // No message to pop
    return false;
}

bool check_prempt(struct prio_queue *pq, struct msg m) {
    if(m.prio == PRIO_HIGH) return false;
    if(m.prio == PRIO_MED) {
        if(!is_empty(&(pq->queues[PRIO_HIGH]))) return true;
    }
    if(m.prio == PRIO_LOW) {
        if(!is_empty(&(pq->queues[PRIO_HIGH])) &&
            !is_empty(&(pq->queues[PRIO_MED]))
        ) return true;
    }
    return false;
}

bool insert_front(struct prio_queue *pq, struct msg msg_in) {
    if(msg_in.prio != PRIO_HIGH &&
       msg_in.prio != PRIO_MED &&
       msg_in.prio != PRIO_LOW) {
        printf("invalid priority\n");
        return false;
    }  
    if (queue_sr(&(pq->queues[msg_in.prio]))) {
        pq->queues[msg_in.prio].msgs[0] = msg_in;
        return true;
    } else {
        return false;
    }
}

static bool queue_sl(struct queue *q) {
    if(!is_empty(q)) {
        for(int i=0; i < q->count; i++) {
            q->msgs[i] = q->msgs[i+1];
        }
        q->count--;
        return true;
    }
    return false;
}

static bool queue_sr(struct queue *q) {
    if(!is_full(q) && !is_empty(q)) {
        for(int i=0; i < q->count; i++) {
            q->msgs[i+1] = q->msgs[i];
        }
        q->count++;
        return true;
    }
    return false;
}

int get_count(struct prio_queue *pq) {
    int ret = 0;
    for(int i = 0; i < NUM_PRIO; i++) {
        ret += pq->queues[i].count;
    }
    return ret;
}

void print_queue(struct prio_queue *pq) {
    for(int i = 0; i < NUM_PRIO; i++) {
        printf("queue prio = %d", i);
        printf(", queue count = %d\n", pq->queues[i].count);
        for(int j = 0; j < pq->queues[i].count; j++) {
            printf("val %d = %d\n", j, pq->queues[i].msgs[j].val);
        }
    }
}