
#ifndef __HOST_H__
#define __HOST_H__
#include <pthread.h>
/*
typedef struct packet_contents{
    int flow;
    int dest;
}packet; */

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


extern int socketfd[16];
extern int interfaceindex[16];

extern pthread_t senddemands_thread, recievedemands_thread, sendpackets_thread;
extern pthread_cond_t queue_not_empty;
extern FILE *traffic_file;
extern short host_number;
extern FILE *send_dmd_fp, *recv_dmd_fp, *send_pkts_fp, *transmit_pkts_fp;

extern void command_parser(int *sockfd);
extern int recieve_demands(int *sockfd);
extern int send_packets(int *sockfd);

extern int open_socket(int index, int *rifindex);



#endif
