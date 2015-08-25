#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <queue.h>
#include <arbiter.h>
#include <signal.h>

// TIME SLOT locking stuff
short ts_complete = 0;
pthread_mutex_t ts_complete_lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ts_lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ts_finished = PTHREAD_COND_INITIALIZER;

// =======================================================================================
pthread_t accept_connections_thread, recieve_request_thread[16], process_request_thread, send_response_thread, signal_thread;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t res_que_notempty = PTHREAD_COND_INITIALIZER;

pthread_mutex_t response_qlock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t sockfd_lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t req_qlock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t res_qlock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t out_chunk_lock5 = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t out_chunk_lock6 = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t out_chunk_lock7 = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

host_req_queue host_reqQ_5000[17]; // Stores start, end and length of 16 Q's
host_req_queue host_reqQ_6000[17]; // Stores start, end and length of 16 Q's
host_req_queue host_reqQ_7000[17]; // Stores start, end and length of 16 Q's
remaining_chunks outstanding_chunks_5000[17]; // Stores remaining chunks for a host 
remaining_chunks outstanding_chunks_6000[17]; // Stores remaining chunks for a host 
remaining_chunks outstanding_chunks_7000[17]; // Stores remaining chunks for a host 

host_req_queue hostreq_Q5000;
host_req_queue hostreq_Q6000;
host_req_queue hostreq_Q7000;

host_req_queue response_queue;

// ====================================
request_array request_7000;
request_array request_5000;
request_array request_6000;
pthread_mutex_t no_req_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t no_req_cond = PTHREAD_COND_INITIALIZER;


host_reply reply[240];
int reply_index;


//====================================
short hostsockfd[17];
fd_set hostsockfdset;
short biggestfd;
FILE *accept_con_fp, *recv_dmd_fp, *send_dmd_fp;
short create_server_socket();
void open_log_files();
void close_log_files();


sigset_t set;
struct sigaction act;
int *sig_th_status;
int sig;
int result = -1;


void
signal_handler(int *sockfd){
	int result;
	int i;
	result = sigwait(&set,&sig);	//--- UBUNTU
	//result = sigwait(&set); // 

	//printf("\n SIGNAL THREAD\n");
	//fflush(stdout);
	if(result == 0){  // ------ UBUNTU
		printf("\n Control+C pressed ......... Arbiter stopping\n");
		pthread_cancel(accept_connections_thread);
		for(i=0; i<16; i++)
		pthread_cancel(recieve_request_thread[i]);
		pthread_cancel(process_request_thread);
		close(*sockfd);
		close_log_files();
		printf("\n SOCKET CLOSED !!!\n");
		exit(1);
	}else{
		printf("\n Error occurred after pressing <cntrl+c>");	
	}
	return;
}


void
hostsockfd_init(){
	short i;
	for(i = 1; i <= 16; i++){
		hostsockfd[i] = 0;
	}
}

void
init_host_req_queues(){
	int i = 0;
	for(i = 1; i <= 16; i++){
		host_reqQ_5000[i].start = NULL; host_reqQ_6000[i].start = NULL; host_reqQ_7000[i].start = NULL;		
		host_reqQ_5000[i].end = NULL; host_reqQ_6000[i].end = NULL; host_reqQ_6000[i].end = NULL;		
		host_reqQ_5000[i].qlen = 0; host_reqQ_6000[i].qlen = 0; host_reqQ_7000[i].qlen = 0;
		host_reqQ_5000[i].lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
		host_reqQ_6000[i].lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
		host_reqQ_7000[i].lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
		outstanding_chunks_5000[i].host = i;		
		outstanding_chunks_5000[i].chunks_left = 0;		
		outstanding_chunks_6000[i].host = i;		
		outstanding_chunks_6000[i].chunks_left = 0; 
		outstanding_chunks_7000[i].host = i;		
		outstanding_chunks_7000[i].chunks_left = 0;	
	}
//	host_req_Q5000.start = NULL; host_req_Q5000.end = NULL; host_req_Q5000.qlen = 0; host_req_Q5000.lock = PTHREAD_MUTEX_INITIALIZER;
//	host_req_Q6000.start = NULL; host_req_Q6000.end = NULL; host_req_Q6000.qlen = 0; host_req_Q6000.lock = PTHREAD_MUTEX_INITIALIZER;
//	host_req_Q7000.start = NULL; host_req_Q7000.end = NULL; host_req_Q7000.qlen = 0; host_req_Q7000.lock = PTHREAD_MUTEX_INITIALIZER;


	memset(request_7000.request, 0, sizeof(host_demand)*240);
	request_7000.len = 0;
	request_7000.lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

	memset(request_5000.request, 0, sizeof(host_demand)*240);
	request_5000.len = 0;
	request_5000.lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

	memset(request_6000.request, 0, sizeof(host_demand)*240);
	request_6000.len = 0;
	request_6000.lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

}

int
main(){
	void **value_ptr = NULL;
	short host_numbers[16];
    	short sockfd = create_server_socket();
	int i;
    	if(sockfd < 0){
    		printf("\n Arbiter Error. Arbiter closing ...\n");
    		return -1;
    	}

    	hostsockfd_init();

    	sigemptyset(&set);	
	sigaddset(&set, SIGINT);
	pthread_sigmask(SIG_BLOCK, &set, NULL);
	//printf("\n Going to Signal Handling .....\n");
	pthread_create(&signal_thread, 0, (void *)&signal_handler, &sockfd);

	open_log_files();
    	//printf("\n accept_connections thread ---------------\n");
   	pthread_create(&accept_connections_thread, 0, (void *)&accept_connections, &sockfd);
   	//printf("\n get_demands thread -------------------");
	for(i=0; i<16; i++){
		host_numbers[i] = i+1;	
        	pthread_create(&recieve_request_thread[i], 0, (void *)&get_demands, &(host_numbers[i]));
	}
        	//printf("\n process_demands thread ----------------------");
        	pthread_create(&process_request_thread, 0, (void *)&process_demands, NULL);
        //	pthread_create(&send_response_thread, 0, (void *)&send_demands_to_host, NULL);
        	
        	
        	
        	//pthread_create(&send_response_thread, 0, (void *)&get_demands, NULL);

    	pthread_join (accept_connections_thread, value_ptr);
	for(i=0; i<16; i++)
     	 pthread_join (recieve_request_thread[i], value_ptr);	
     //	pthread_join (process_request_thread, value_ptr);
     //	pthread_join (send_response_thread, value_ptr);
     	//close_log_files();	
     	return 0;
}

short 
create_server_socket() {

	struct sockaddr_in serv_addr;
	int portno;
	short sockfd = socket(AF_INET, SOCK_STREAM, 0);
	printf("\n SOCKFD: %d", sockfd);
//	fcntl(sockfd, F_SETFL, O_NONBLOCK);
	if (sockfd < 0) {
		printf("\n Error opening socket at server");
		return -1;
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = 5000;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("20.0.0.100");
	//serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(portno);

	int yes = 1;
	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) { 
	    printf("\n setsockopt error!"); 
	    return -1;
	} 

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("\n Error while binding socket");
		return -1;
	}
	/* Maximum 18 connections allowed with arbiter. */
	if(listen(sockfd,18) < 0){
		printf("\n Listen Failed");
		return -1;
	}
	return sockfd;
}

void
open_log_files(){
	accept_con_fp = fopen("log/arbiter-accept-connections.log", "w");
	if(accept_con_fp == NULL)
		printf("\n Cannot Create arbiter-accept-connections log file");
	recv_dmd_fp = fopen("log/arbiter-recieve-demands.log", "w");
	if(recv_dmd_fp == NULL)
		printf("\n Cannot Create arbiter-recieve-demands log file");
	send_dmd_fp = fopen("log/arbiter-send-demands.log", "w");
	if(send_dmd_fp == NULL)
		printf("\n Cannot Create arbiter-send-demands log file");	
}
void
close_log_files(){
	
	fclose(accept_con_fp);
	fclose(recv_dmd_fp);
	fclose(send_dmd_fp);
}
