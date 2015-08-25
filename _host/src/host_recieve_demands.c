#include <stdio.h>
#include <host.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <queue.h>
/*
typedef struct demand_qnode_data{
    	short src;
	short dest;
	short flow;
	short vlanid;
}demand_qnode_data;
*/

int
recieve_demands(int *sockfd){
	//host_demand demand;
//	demand_qnode_data demand;
	demand_qnode_data *data;
	host_reply demand;
	int n;
	int count = 0;
	//printf("\n SOCKET AT HOST : %d", *sockfd);
	init_demand_queue();
	while(1){
		fflush(stdout);
		n = read(*sockfd, (void *)&demand, sizeof(struct host_reply));
		if(n == 0){
			printf("\n Breaking ............................\n");
			break;
		}
		if(n < 0){
			printf("\n Recieve-demands-thread : Error reading from socket. Thread Returning");
			return -1;
		}
		fprintf(recv_dmd_fp, "Timeslot: %d  Src: %hd    Dest: %hd   Flow: %hd  vlanid: %hd  Port: %d\n",demand.timeslot, demand.src, demand.dest, demand.flow, demand.vlanid, demand.port);
	//	printf("\nFrom Arbiter : Src: %hd    Dest: %hd   Flow: %hd  vlanid: %hd  Port: %d", demand.src, demand.dest, demand.flow, demand.vlanid, demand.port);
		data = (demand_qnode_data *)malloc(sizeof(demand_qnode_data));
		data->timeslot = demand.timeslot;
		data->src = demand.src;
		data->dest = demand.dest;
		data->flow = demand.flow;
		data->port = demand.port;
		data->vlanid = demand.vlanid;
		enqueue_demand(data);
		if(queue_size() == 1)
			pthread_cond_signal(&queue_not_empty);	
		//usleep(10000);	
		count++;
	//	printf("\n count-%d = %d\n", demand.src, count);*/
		fflush(recv_dmd_fp);
	}
	printf("\n RECIEVE DEMANDS RETURNING\n");
	return 1;
	
}
