
#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <pthread.h>

typedef struct demand_qnode_data{
    	short src;
	short dest;
	short port;
	short flow;
	short vlanid;
}demand_qnode_data;

// Queue on Arbiter
typedef struct demand_qnode{
	struct demand_qnode *prev;
    	struct demand_qnode_data *data;	
    	struct demand_qnode *next;
}demand_qnode;

typedef struct host_req_queue{
	int qlen;
	demand_qnode *start;
	demand_qnode *end;
	pthread_mutex_t lock;
}host_req_queue;

extern pthread_mutex_t queue_lock;
extern pthread_mutex_t req_qlock;
extern pthread_mutex_t res_qlock;
extern pthread_mutex_t response_qlock;


extern host_req_queue host_reqQ_5000[17]; // Stores start, end and length of 16 Q's
extern host_req_queue host_reqQ_6000[17]; // Stores start, end and length of 16 Q's
extern host_req_queue host_reqQ_7000[17]; // Stores start, end and length of 16 Q's


extern host_req_queue response_queue;

extern host_req_queue hostreq_Q5000;
extern host_req_queue hostreq_Q6000;
extern host_req_queue hostreq_Q7000;


extern void init_demand_queue();
extern short is_demand_qempty();
inline extern int queue_size();
extern void free_demand_queue();
extern void enqueue_demand(demand_qnode_data *data, host_req_queue *queue);
extern void dequeue_demand(demand_qnode_data *data, host_req_queue *queue);
extern void peep_dequeue_demand(demand_qnode_data *ret_data, host_req_queue *queue);
extern void print_demand_queue();

#endif
