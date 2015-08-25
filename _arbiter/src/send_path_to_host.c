
#include <arbiter.h>
#include <queue.h>
#include <unistd.h>
int count = 0;
void
send_demands_to_host(){
	printf("\n RESPONSE QUEUE SIZE : %d", response_queue.qlen);
	int n, len;
	demand_qnode_data data;
	len = response_queue.qlen;
	while(1){
	/*	if(response_queue.qlen == 0){
			printf("\n>>>>>>>>>>> Send Response Thread sleeping");
			fflush(stdout);
			pthread_mutex_lock(&response_qlock);
			pthread_cond_wait(&queue_not_empty, &response_qlock);
			pthread_mutex_unlock(&response_qlock);
			printf("\n<<<<<<<<<<< Send Response Thread waking up !!!");
		} */
		fflush(stdout);
		data.src = -1;
		dequeue_demand(&data, &response_queue);
		if(data.src == -1){
			usleep(100);
			continue;
		}
		n = write(hostsockfd[data.src],(void *)&data, sizeof(demand_qnode_data));
   		if (n < 0) 
   			printf("\n Error: Cannot send demamd response to host h%d on sockfd %d", data.src, hostsockfd[data.src]); 
		count ++;
		fflush(stdout);
		//sleep(2);
	}

}
