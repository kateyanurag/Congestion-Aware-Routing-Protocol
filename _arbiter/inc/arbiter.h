
#include <pthread.h>
#include <stdio.h>
#include <sys/select.h>

#ifndef __DEMAND_PACKET__
#define __DEMAND_PACKET__

typedef struct host_demand{
	short src;
	short dest;
	short port;
	short flow;
	short vlanid;
}host_demand;

typedef struct host_reply{
	int timeslot;
	short src;
	short dest;
	short port;
	short flow;
	short vlanid;
}host_reply;

typedef struct cperf_demand{
	short src;
	short dest;
	short port;
	short flow;
	short ack;
}cperf_demand;

typedef struct remaining_chunk{
	int chunks_left;
	int host;
}remaining_chunks;

typedef struct request_array{
	cperf_demand request[240];
	short len;
	pthread_mutex_t lock;
}request_array;

/*
typedef struct hostsocket{
	int host;
	int sockfd;
}hostsocket; */


extern short ts_complete;
extern pthread_mutex_t ts_lock;
extern pthread_mutex_t ts_complete_lock;
extern pthread_cond_t ts_finished;

extern pthread_t accept_connections_thread, recieve_request_thread[16], process_request_thread, send_response_thread, signal_thread;
extern pthread_cond_t queue_not_empty;
extern pthread_cond_t res_que_notempty;
extern pthread_mutex_t sockfd_lock;


extern pthread_mutex_t out_chunk_lock5;
extern pthread_mutex_t out_chunk_lock6;
extern pthread_mutex_t out_chunk_lock7;


extern short hostsockfd[17];
extern short biggestfd;
extern fd_set hostsockfdset;

extern FILE *accept_con_fp;
extern FILE *recv_dmd_fp;
extern FILE *send_dmd_fp;


extern remaining_chunks outstanding_chunks_5000[17]; // Stores remaining chunks for a host 
extern remaining_chunks outstanding_chunks_6000[17]; // Stores remaining chunks for a host 
extern remaining_chunks outstanding_chunks_7000[17]; // Stores remaining chunks for a host 


extern request_array request_7000;
extern request_array request_5000;
extern request_array request_6000;
extern pthread_cond_t no_req_cond;
extern pthread_mutex_t no_req_mutex;

extern host_reply reply[240];
extern int reply_index;

extern void accept_connections(short *sockfd);	
extern void get_demands(short *host_num);
extern void process_demands();
extern void send_demands_to_host();
 
 #endif
