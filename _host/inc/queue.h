
#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <pthread.h>

typedef struct demand_qnode_data{
    	int timeslot;
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


extern pthread_mutex_t req_qlock;
extern pthread_mutex_t response_qlock;

extern void init_demand_queue();
inline extern int queue_size();
extern short is_demand_qempty();
extern void free_demand_queue();
extern void enqueue_demand(demand_qnode_data *data);
extern void dequeue_demand(demand_qnode_data *data)		;
extern void print_demand_queue();

#endif
