#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <stdlib.h>
#include <arbiter.h>
#include <queue.h>
#include <math.h>
#include <pthread.h> 

float
get_random_sleep(){
 // int c;
  float n;
  //printf("Ten random numbers \n");
 
 // for (c = 1; c <= 1000; c++) {
    n = (float)rand()/pow(10, 11);
  //  printf("%f\n", n);
 // }
 
  return n;
}

demand_qnode_data*
read_from_socket(short sockfd){
	int n;
	demand_qnode_data *data = NULL;
	host_demand demand;
	if(sockfd != 0){
		n = read(sockfd, (void *)&demand,sizeof(host_demand));
		if(n > 0){
			data = (demand_qnode_data *)malloc(sizeof(demand_qnode_data));
			data->flow = demand.flow;
			data->dest = demand.dest;
			data->src = demand.src;
			data->port = demand.port;
			data->vlanid = 100;
		}
	}
	return data;	
}

void
init_fdset(){
	int i;
	FD_ZERO(&hostsockfdset);
	for (i = 1; i <= 16; i++) {
		if(hostsockfd[i] != 0)
        		FD_SET(hostsockfd[i], &hostsockfdset);
    	}
}

void
store_demands(request_array *req_arr, cperf_demand demand){
	pthread_mutex_lock(&req_arr->lock);
	req_arr->request[req_arr->len].src = demand.src;
	req_arr->request[req_arr->len].dest = demand.dest;
	req_arr->request[req_arr->len].flow = demand.flow;
	req_arr->request[req_arr->len].port = demand.port;
	req_arr->len++;
	pthread_mutex_unlock(&req_arr->lock);
	fprintf(recv_dmd_fp, "Recieved from h%hd  Dest : h%hd  Flow: %hd  Port: %d  Ack: %d\n", demand.src, demand.dest, demand.flow, demand.port, demand.ack);
	//printf("\n Recieved from h%hd  Dest : h%hd  Flow: %hd  Port: %d  Ack: %d", demand.src, demand.dest, demand.flow, demand.port, demand.ack);
	fflush(recv_dmd_fp);
	//fflush(stdout);
}

void
print_request_array(request_array req_arr){
	int i = 0;
	cperf_demand demand;
	printf("\n Req_arrlen:%d ----------------------------------------------------------------------------------------------------", req_arr.len);
	for(i=0; i<req_arr.len; i++){
		demand = req_arr.request[i];
		printf("\n Src:%d  Dest:%d Flow:%d Port:%d Ack:%d", demand.src, demand.dest, demand.flow, demand.port, demand.ack);
	}	
	fflush(stdout);
}

short
pending_requests(){
	pthread_mutex_lock(&request_5000.lock);
	pthread_mutex_lock(&request_6000.lock);
	pthread_mutex_lock(&request_7000.lock);
	if(request_5000.len - 1 == 0 || request_6000.len - 1 == 0  || request_7000.len - 1 == 0){
		pthread_cond_signal(&no_req_cond);
	}
	pthread_mutex_unlock(&request_7000.lock);
	pthread_mutex_unlock(&request_6000.lock);
	pthread_mutex_unlock(&request_5000.lock);
	return 0;
}

void
get_demands(short *host_num){
	cperf_demand demand;
	int n;
	printf("\n In get demands : Recv-Thread%d", *host_num);
	while(1){
	///	printf("\n YESSSSSSSSSSSSSSSSSSSSSSS    HOST NUM: %d", *host_num);
		if(hostsockfd[*host_num] != 0){
		//	printf("\n YESSSSSSSSSSSSSSSSSSSSSSS    HOST NUM: %d", *host_num);
		//	fflush(stdout);
			n = read(hostsockfd[*host_num],(void *)&demand,sizeof(host_demand));
			if(n == 0){
				//i = (i + 1)%16;
				continue;
			}
			if(n < 0){
			//	i = (i + 1)%16;
				continue;
			}
			/*if(demand.ack){
				printf("\n THIS IS AN ACK PACKET");
			}*/
			if(demand.port / 1000 == 5){
			//printf("\n ================== TRUE");
				store_demands(&request_5000, demand);
				//print_request_array(request_5000);
				pthread_cond_signal(&no_req_cond);
			}
			if(demand.port / 1000 == 6){
				store_demands(&request_6000, demand);
				pthread_cond_signal(&no_req_cond);
			}
			if(demand.port / 1000 == 7){
				store_demands(&request_7000, demand);
				pthread_cond_signal(&no_req_cond);
			}
			if(demand.ack == 1){	
		fprintf(recv_dmd_fp, "ACK  - Recieved from h%hd  Dest : h%hd  Flow: %hd  Port: %d  Ack: %d\n", demand.src, demand.dest, demand.flow, demand.port, demand.ack);
				pthread_mutex_lock(&ts_complete_lock);
				ts_complete = ts_complete & ~(1 << (demand.src-1));
				if(ts_complete == 0){
					fprintf(recv_dmd_fp, "COMPLETE TIMESLOT : %d - Recieved from h%hd  ",demand.flow, demand.src);
					pthread_cond_signal(&ts_finished);
				}
				pthread_mutex_unlock(&ts_complete_lock);
			}
		
		}else{
			sleep(2);
		}
	}

}


/*
void
get_demands(){
	short i = 1;
	short flag = 0;
	int retval = 0;
	struct timeval tv;
	int prev_timeval = 0;
	demand_qnode_data *data = NULL;
	//tv.tv_sec = 1;
	tv.tv_usec = 500000;
	int count = 1; 
	int host_num;
	printf("\n ************* GET DEMANDS FROM HOST THREAD : \n");
	while(1){
		if(hostsockfd[i] != 0){
			n = read(hostsockfd[i],(void *)&demand,sizeof(host_demand));
			if(n == 0){
				i = (i + 1)%16;
				continue;
			}
			if(n < 0){
				i = (i + 1)%16;
				continue;
			}
			data = (demand_qnode_data *)malloc(sizeof(demand_qnode_data));
			data->flow = demand.flow;
			data->dest = demand.dest;
			data->src = demand.src;
			data->port = demand.port;
			data->vlanid = 100;

			
			//pthread_mutex_lock(&req_qlock);
			enqueue_demand(data);
			//pthread_mutex_unlock(&req_qlock);


			usleep(5000);
			fprintf(recv_dmd_fp, "Recieved from h%hd    Dest : h%hd    Flow: %hd    Vlanid: %hd  Port: %d\n", demand.src, demand.dest, demand.flow, demand.vlanid, demand.port);
			fflush(recv_dmd_fp);
		} 
	

		
		init_fdset();
		prev_timeval = tv.tv_sec; 
		retval = select(biggestfd + 1, &hostsockfdset, NULL, NULL, &tv);
		if(retval < 0){
			printf("\n SELECT FAILED !!!");
			exit(0);
		}
		if(retval == 0){
			fflush(recv_dmd_fp);
			//tv.tv_sec = prev_timeval + 2; 
		//	tv.tv_sec = 1; 
			tv.tv_usec = 500000;
		//	printf("\n --------------- NO DATA in select !!!!!!!!!!!! ------------------ %ld", tv.tv_sec);
			continue;
		}
		if(retval > 0){
			for(i = 1; i <= 16; i++){
				if(FD_ISSET(hostsockfd[i], &hostsockfdset)){
					data = read_from_socket(hostsockfd[i]);
					count++;
					fflush(stdout);
					if(data){
						flag = 0;
						fflush(recv_dmd_fp);
						host_num = data->src;
						if(data->port / 1000 == 5){
							fprintf(recv_dmd_fp, "**PORT: %d   Recieved from h%hd    Dest : h%hd    Flow: %hd    Vlanid: %hd  Port: %d\n", data->port,data->src, data->dest, data->flow, data->vlanid, data->port);
							printf("\n 5000 Request from h%d", data->src);
							fflush(recv_dmd_fp);
							enqueue_demand(data, &host_reqQ_5000[host_num]);
							flag = 1;
							pthread_cond_signal(&queue_not_empty);
							if (host_reqQ_5000[host_num].qlen -1 == 0){
								printf("\n Signalling 5000 cond!");
								pthread_cond_signal(&queue_not_empty);
							}
						}
						if(data->port / 1000 == 6){
							fprintf(recv_dmd_fp, "***PORT: %d   Recieved from h%hd    Dest : h%hd    Flow: %hd    Vlanid: %hd  Port: %d\n", data->port,data->src, data->dest, data->flow, data->vlanid, data->port);
							enqueue_demand(data, &host_reqQ_6000[host_num]);
							flag = 1;
							pthread_cond_signal(&queue_not_empty);
							if (host_reqQ_6000[host_num].qlen - 1 == 0){
								printf("\n Signalling 6000 cond!");
								pthread_cond_signal(&queue_not_empty);
							}
						}
						if(data->port / 1000 == 7){
							fprintf(recv_dmd_fp, "***PORT: %d   Recieved from h%hd    Dest : h%hd    Flow: %hd    Vlanid: %hd  Port: %d\n", data->port,data->src, data->dest, data->flow, data->vlanid, data->port);
							enqueue_demand(data, &host_reqQ_7000[host_num]);
							flag = 1;
							pthread_cond_signal(&queue_not_empty);
							if (host_reqQ_7000[host_num].qlen - 1 == 0){
								printf("\n Signalling 7000 cond!");
								pthread_cond_signal(&queue_not_empty);
							}
						}
						if(flag)
							pthread_cond_signal(&queue_not_empty);
						flag = 0;
						//if(queue_size() == 1)
						// commented now !
						//usleep(5000);
						tv.tv_usec = 500000;
						fflush(stdout);
					}else{
						tv.tv_usec = 500000;
					}					
				}
			}
		
		}	
		fflush(stdout);
	}
}
*/
