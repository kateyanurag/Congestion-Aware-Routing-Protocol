#include <arbiter.h>
#include <queue.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef struct timeslot_vlanid{
	short vid;
	int flow;
}timeslot_vlanid;

short prev_timeslot_paths[17];
short which_flow = 0;
int TIME_SLOT = 1;


short pod_vlanid[4][4]; 		// Aggregate Switch : Pod vs VlanID array -- storing flow count(max 200) for each VLANID in each pod in a particular timeslot.
short torswitch_port_capacity[8][2]; 	// 8 TOR switches and 2 ports viz. port1 and port2.
short tor_link_capacity[8][16];		// 8 TOR switches and 2 links/ports directly connected to hosts.
short coreswitch_port_capacity[4][4];	// Core Switch : 4 core switches and each vlanid corresponds to each port.

short host_to_switch[] = {0, 7, 7, 8, 8, 11, 11, 12, 12, 15, 15, 16, 16, 19, 19, 20, 20};

short prev_vlanid[17][17];

remaining_chunks chunks_left_5000[17];
remaining_chunks chunks_left_6000[17];
remaining_chunks chunks_left_7000[17];




timeslot_vlanid prev_timeslot_vlanid[17][4];
timeslot_vlanid curr_timeslot_vlanid[17][4];


void send_demands_to_host();


demand_qnode_data*
get_queue_node(demand_qnode_data d){
	
	demand_qnode_data *data = NULL;
	data = (demand_qnode_data *)malloc(sizeof(demand_qnode_data));
	data->flow = d.flow;
	data->dest = d.dest;
	data->src = d.src;
	data->port = d.port;
	return data;
}

inline short
in_same_pod(short src, short dest){
	if ((src -1)/4 == (dest - 1)/4)
		return 1;
	return 0;
}

inline short
are_sibblings(short src, short dest){
	if(host_to_switch[src] == host_to_switch[dest])
		return 1;
	return 0;	
}	



short
get_vlanid(short src, short dest, short flow, short same_pod, short temp_vlanid){

	int src_switch_idx, dest_switch_idx;                
	if(dest % 2 == 0)
		dest_switch_idx = (dest-1) / 2 - 1;
	if(dest % 2 != 0)
		dest_switch_idx = (dest) / 2 - 1;
	if(src % 2 == 0)
		src_switch_idx = (src-1) / 2 - 1;
	if(src % 2 != 0)
		src_switch_idx = (src) / 2 - 1;
	if(src_switch_idx == -1)
		src_switch_idx = 0;	

	if(same_pod){	
		// Check if VlanID 0xxx can be allocated.
		if(temp_vlanid < 1000){
			if(pod_vlanid[(src-1)/4][0] + flow <= 200 && pod_vlanid[(dest-1)/4][0] + flow <= 200){
				if(torswitch_port_capacity[src_switch_idx][0] + flow <= 200 && torswitch_port_capacity[dest_switch_idx][0] + flow <= 200){
					torswitch_port_capacity[src_switch_idx][0] += flow;
					torswitch_port_capacity[dest_switch_idx][0] += flow;
					pod_vlanid[(src-1)/4][0] += flow;
					pod_vlanid[(dest-1)/4][0] += flow;
					return (src << 4)+dest;
				}else {return -1;}
			}else {return -1;}
		}
		// Check if VlanID 1xxx can be allocated.
		if(temp_vlanid < 2000){	
			if(pod_vlanid[(src-1)/4][1] + flow <= 200 && pod_vlanid[(dest-1)/4][1] + flow <= 200){
				if(torswitch_port_capacity[src_switch_idx][1] + flow <= 200 && torswitch_port_capacity[dest_switch_idx][1] + flow <= 200){
					torswitch_port_capacity[src_switch_idx][0] += flow;
					torswitch_port_capacity[dest_switch_idx][0] += flow;
					pod_vlanid[(src-1)/4][1] += flow;
					pod_vlanid[(dest-1)/4][1] += flow;
					return ((src << 4)+dest)+1000;
				}else {return -1;}
			}else {return -1;}
		}
	}
	if(!same_pod){
		// Check if VlanID 0xxx can be allocated.
		if(temp_vlanid < 1000){	
			if(coreswitch_port_capacity[0][0] + flow <= 200){ // switch S1 and 0xxx vlanID
				if(pod_vlanid[(src-1)/4][0] + flow <= 200 && pod_vlanid[(dest-1)/4][0] + flow <= 200){
					if(torswitch_port_capacity[src_switch_idx][0] + flow <= 200 && torswitch_port_capacity[dest_switch_idx][0] + flow <= 200){
						torswitch_port_capacity[src_switch_idx][0] += flow;
						torswitch_port_capacity[dest_switch_idx][0] += flow;
						pod_vlanid[(src-1)/4][0] += flow;
						pod_vlanid[(dest-1)/4][0] += flow;
						coreswitch_port_capacity[0][0] += flow;
						return (src << 4)+dest;
					} else {//if(TIME_SLOT == 4) printf("\n Byee1");
						return -1;}
				} else {//if(TIME_SLOT == 4) printf("\n Byee2");
					return -1;}
			}else {//if(TIME_SLOT == 4) printf("\n BYEEE3");
				return -1;}
		}
		// Check if VlanID 1xxx can be allocated.
		if(temp_vlanid < 2000){	
			if(coreswitch_port_capacity[2][1] + flow <= 200){ // switch S3 and 1xxx vlanID
				if(pod_vlanid[(src-1)/4][1] + flow <= 200 && pod_vlanid[(dest-1)/4][1] + flow <= 200){
					if(torswitch_port_capacity[src_switch_idx][1] + flow <= 200 && torswitch_port_capacity[dest_switch_idx][1] + flow <= 200){
						torswitch_port_capacity[src_switch_idx][0] += flow;
						torswitch_port_capacity[dest_switch_idx][0] += flow;
						pod_vlanid[(src-1)/4][1] += flow;
						pod_vlanid[(dest-1)/4][1] += flow;
						coreswitch_port_capacity[2][1] += flow;
						return ((src << 4)+dest)+1000;
					} else {return -1;}
				} else {return -1;}
			} else {return -1;}
		}

		// Check if VlanID 2xxx can be allocated.
		if(temp_vlanid < 3000){	
			if(coreswitch_port_capacity[1][2] + flow <= 200){ // switch S2 and 2xxx vlanID
				if(pod_vlanid[(src-1)/4][0] + flow <= 200 && pod_vlanid[(dest-1)/4][0] + flow <= 200){
					if(torswitch_port_capacity[src_switch_idx][0] + flow <= 200 && torswitch_port_capacity[dest_switch_idx][0] + flow <= 200){
						torswitch_port_capacity[src_switch_idx][0] += flow;
						torswitch_port_capacity[dest_switch_idx][0] += flow;
						pod_vlanid[(src-1)/4][0] += flow;
						pod_vlanid[(dest-1)/4][0] += flow;
						coreswitch_port_capacity[1][2] += flow;
						return ((src << 4)+dest)+2000;
					} else {return -1;}
				} else {return -1;}
			} else {return -1;}
		}
		// Check if VlanID 3xxx can be allocated.
		if(temp_vlanid < 4000){	
			if(coreswitch_port_capacity[3][3] + flow <= 200){ // switch S4 and 3xxx vlanID
				if(pod_vlanid[(src-1)/4][1] + flow <= 200 && pod_vlanid[(dest-1)/4][1] + flow <= 200){
					if(torswitch_port_capacity[src_switch_idx][1] + flow <= 200 && torswitch_port_capacity[dest_switch_idx][1] + flow <= 200){
						torswitch_port_capacity[src_switch_idx][0] += flow;
						torswitch_port_capacity[dest_switch_idx][0] += flow;
						pod_vlanid[(src-1)/4][1] += flow;
						pod_vlanid[(dest-1)/4][1] += flow;
						coreswitch_port_capacity[3][3] += flow;
						return ((src << 4)+dest)+3000;
					} else {return -1;}
				} else {return -1;}
			} else {return -1;}
		}
	}
	return -1;		
	
}



// Circularly checks for Available Vlan ID's
short
get_appropriate_vlanid(short src, short dest, short flow, short same_pod){

	int vlanid, temp;
	int i;
	temp = prev_vlanid[src][dest];	
	if(same_pod){
		for(i = 0; i<=1; i++){
			if(temp == 0){
				temp = (src<<4) + dest;
				vlanid = get_vlanid(src, dest, flow, same_pod, temp);
				if(vlanid > 0) return vlanid;
			}
			if(temp < 1000){
				temp = temp + 1000;
				vlanid = get_vlanid(src, dest, flow, same_pod, temp);
				if(vlanid > 0) return vlanid;
			}
			if(temp < 2000){
				temp = temp - 1000;
				vlanid = get_vlanid(src, dest, flow, same_pod, temp);
				if(vlanid > 0) return vlanid;
			}	
		}
		return -1;
	}
	
	temp = prev_vlanid[src][dest];
	if(!same_pod){
		for(i = 0; i<=3; i++){
			if(temp == 0){
				temp = (src<<4) + dest;
				vlanid = get_vlanid(src, dest, flow, same_pod, temp);
				if(vlanid > 0) return vlanid;
			}
			if(temp < 1000){
				temp = temp + 1000;
				vlanid = get_vlanid(src, dest, flow, same_pod, temp);
				if(vlanid > 0) return vlanid;
			}if(temp < 2000){
				temp = temp + 1000;
				vlanid = get_vlanid(src, dest, flow, same_pod, temp);
				if(vlanid > 0) return vlanid;
			}if(temp < 3000){
				temp = temp + 1000;
				vlanid = get_vlanid(src, dest, flow, same_pod, temp);
				if(vlanid > 0) return vlanid;
			}if(temp < 4000){	
				temp = temp - 3000;
				vlanid = get_vlanid(src, dest, flow, same_pod, temp);
				if(vlanid > 0) return vlanid;
			}
		}
		return -1;
	}
	return -1;
}



void
//find_path(short host_num, int p){
find_path(int index, int p){
//find_path(int p){
	cperf_demand demand;
	//int i = 1;
	demand_qnode_data data;
	//data.src = 0;
	int dest_index = 0, src_index;
	//data.src = 0;
	int n;
	int dec_flag = 0;
	int flow = 50;
//	printf("\n @@@@@@@@@@ Inside find path");
	// Traverse the queue(list) to assign vlanID's to the demands.
	if(p == 7){
		demand = request_7000.request[index];
		//i = host_reqQ_7000[host_num].qlen;
	}
	else if(p == 6){
		demand = request_6000.request[index];
		//i = host_reqQ_6000[host_num].qlen;
	}
	else if(p == 5){
		demand = request_5000.request[index];
		//i = host_reqQ_5000[host_num].qlen;
	}	
		
	if(demand.dest % 2 == 0)
		dest_index = demand.dest / 2 - 1;
	if(demand.dest % 2 != 0)
		dest_index = (demand.dest + 1) / 2 - 1;
	if(demand.src % 2 == 0)
		src_index = demand.src / 2 - 1;
	if(demand.src % 2 != 0)
		src_index = (demand.src + 1) / 2 - 1;
	data.src = demand.src;
	data.dest = demand.dest;
	data.port = demand.port;
	data.flow = demand.flow;
	fflush(stdout);
	while(1){ 
	/*
		fflush(stdout);
		dequeue_flag = 0;
		if(p == 7){
			peep_dequeue_demand(&data, &host_reqQ_7000[host_num]);
		}
		else if(p == 6){
			peep_dequeue_demand(&data, &host_reqQ_6000[host_num]);
		}
		else if(p == 5){
			peep_dequeue_demand(&data, &host_reqQ_5000[host_num]);
		}
		
		// Queue Empty!
		if(data.src == 0)
			return -1;	
	
		if(data.dest % 2 == 0)
			dest_index = data.dest / 2 - 1;
		if(data.dest % 2 != 0)
			dest_index = (data.dest + 1) / 2 - 1;
		if(data.src % 2 == 0)
			src_index = data.src / 2 - 1;
		if(data.src % 2 != 0)
			src_index = (data.src + 1) / 2 - 1;
	
		*/
		
	
		// Check if there's some capacity(packet carrying capacity) left in the links directly connected to TOR switch and Host. 
		if(tor_link_capacity[dest_index][data.dest -1] + flow <= 200 && tor_link_capacity[src_index][data.src - 1] + flow <=  200){			
			// If Source and Destinatination are siblings simply assign a path
			if(are_sibblings(data.src, data.dest)){
				data.vlanid = (data.src << 4) + data.dest;
				prev_vlanid[data.src][data.dest] = data.vlanid;
				curr_timeslot_vlanid[data.src][0].vid = 0x1;
				curr_timeslot_vlanid[data.dest][0].vid = 0x1;
				curr_timeslot_vlanid[data.src][0].flow += flow;
				curr_timeslot_vlanid[data.dest][0].flow += flow;
				// No need to update pod_vlanid array here !
				
			//	enqueue_demand(get_queue_node(data), &response_queue);
			//	n = write(hostsockfd[data.src],(void *)&data, sizeof(demand_qnode_data));	
			fprintf(send_dmd_fp, "\nTime-Slot: %d  Dequeued to be send to host - Src: h%hd    Dest: h%hd    Flow:%hd   vlanid:%hd   Port: %d", TIME_SLOT, data.src, data.dest, flow, data.vlanid, data.port);
				reply[reply_index].timeslot = TIME_SLOT;
				reply[reply_index].src = data.src;
				reply[reply_index].dest = data.dest;
				reply[reply_index].port = data.port;
				reply[reply_index].flow = flow;
				reply[reply_index].vlanid = data.vlanid;
				reply_index++;
				dec_flag = 1;
			}
			else {
				// If both source and dest not siblings but are in same pod
				if(in_same_pod(data.src, data.dest)){
					data.vlanid = get_appropriate_vlanid(data.src, data.dest, flow, 1);
					if(data.vlanid == -1)
						return;
					prev_vlanid[data.src][data.dest] = data.vlanid;
				}
				// If both source and dest are in different pod
				else if(!in_same_pod(data.src, data.dest)){
					data.vlanid = get_appropriate_vlanid(data.src, data.dest, flow, 0);
					if(data.vlanid == -1)
						return;
					prev_vlanid[data.src][data.dest] = data.vlanid;
				}
				curr_timeslot_vlanid[data.src][data.vlanid/1000].flow += flow;
				curr_timeslot_vlanid[data.dest][data.vlanid/1000].flow += flow;
			//	n = write(hostsockfd[data.src],(void *)&data, sizeof(demand_qnode_data));	
				//enqueue_demand(get_queue_node(data), &response_queue);
			fprintf(send_dmd_fp, "\nTime-Slot: %d  Dequeued to be send to host - Src: h%hd    Dest: h%hd    Flow:%hd   vlanid:%hd   Port: %d  Qsize: %d", TIME_SLOT, data.src, data.dest, flow, data.vlanid, data.port, response_queue.qlen);
				reply[reply_index].timeslot = TIME_SLOT;
				reply[reply_index].src = data.src;
				reply[reply_index].dest = data.dest;
				reply[reply_index].port = data.port;
				reply[reply_index].flow = flow;
				reply[reply_index].vlanid = data.vlanid;
				reply_index++;
				dec_flag = 1;
			}
			if(dec_flag){
			//	fprintf(send_dmd_fp, "\n Incrementing tor_link_capacity");
				tor_link_capacity[dest_index][data.dest -1] += flow;
				tor_link_capacity[src_index][data.src -1] += flow;
				fflush(send_dmd_fp);
				//goto END;
				// ACTUAL DEQUEUE !!!!!
				if(p == 7){
					request_7000.request[index].flow--;
					if(request_7000.request[index].flow == 0)
						break;
				//	dequeue_demand(&data, &host_reqQ_5000[host_num]);
				}
				else if(p == 5){
					request_5000.request[index].flow--;
					if(request_5000.request[index].flow == 0)
						break;
					//dequeue_demand(&data, &host_reqQ_5000[host_num]);
				}
				else if(p == 6){
					request_6000.request[index].flow--;
					if(request_6000.request[index].flow == 0)
						break;
					//dequeue_demand(&data, &host_reqQ_5000[host_num]);
				}
				
			}
			
			/*n = write(hostsockfd[data.src],(void *)&data, sizeof(demand_qnode_data));
   			if (n < 0) 
   				printf("\n Error: Cannot send demamd response to host h%d on sockfd %d", data.src, hostsockfd[data.src]); 
			*/
			//i--; // While loop counter
			dec_flag = 0;
		} // endif
		else{
		//	data.vlanid = -100;
		//	n = write(hostsockfd[data.src],(void *)&data, sizeof(demand_qnode_data));	
			break;
		}
		fflush(stdout);
	}// end while
	return;
		
}
void
sort_chunks_array(remaining_chunks chunks_arr[], int size){
	int i, j;
	int chunks_left, host;
	for(i = 2; i < size; i++){
      		chunks_left = chunks_arr[i].chunks_left;
      		host = chunks_arr[i].host;
      		j = i - 1;
     	 	while((chunks_left < chunks_arr[j].chunks_left) && (j >= 0)){
			chunks_arr[j+1].chunks_left = chunks_arr[j].chunks_left;
			chunks_arr[j+1].host = chunks_arr[j].host;
          		j = j - 1;
      		}
		chunks_arr[j+1].chunks_left = chunks_left;
		chunks_arr[j+1].host = host;	
  	}
}

void
sort_req_arr(request_array *a, int len){
	int i, j, temp;
	for(i=0; i<len; i++){
		for(j=i+1; j<len; j++){
			if((a->request[i].flow == 0 && a->request[i].flow < a->request[j].flow) || (a->request[j].flow != 0 && a->request[i].flow > a->request[j].flow)){
				temp=a->request[i].src ;a->request[i].src = a->request[j].src ;a->request[j].src =temp;
				temp=a->request[i].dest ;a->request[i].dest = a->request[j].dest ;a->request[j].dest =temp;
				temp=a->request[i].flow ;a->request[i].flow = a->request[j].flow ;a->request[j].flow =temp;
				temp=a->request[i].port ;a->request[i].port = a->request[j].port ;a->request[j].port =temp;
			}
		}
	}	
}

void
adjust_req_arr(request_array *a, int i, int j){
	//int i, j;
//	pthread_mutex_lock(&a->lock);
	while(i < j){
		a->request[i].src = a->request[i+1].src;
		a->request[i].dest = a->request[i+1].dest;
		a->request[i].flow = a->request[i+1].flow;
		a->request[i].port = a->request[i+1].port;
		i++;
	}		
	a->len--;
//	pthread_mutex_unlock(&a->lock);
}

void
print_array(request_array a, int len){
	int i;
	
	for(i = 0;i < len; i++){
		printf("  (%d:%d:%d)", a.request[i].src ,a.request[i].flow, a.request[i].port);
	}
	printf("\n");
	fflush(stdout);
}

void
get_src_dest_index(short src, short dest, short *src_index, short *dest_index){
		if(dest % 2 == 0)
			*dest_index = dest / 2 - 1;
		if(dest % 2 != 0)
			*dest_index = (dest + 1) / 2 - 1;
		if(src % 2 == 0)
			*src_index = src / 2 - 1;
		if(src % 2 != 0)
			*src_index = (src + 1) / 2 - 1;
}

void
process_demands() {
	//demand_qnode_data data;
	//data.src = 0;
	printf("\n ************* PROCESS DEMANDS THREAD: ");
	response_queue.start = NULL;
	response_queue.end = NULL;
	response_queue.qlen = 0;
	
	short q5_empty = 0, q6_empty = 0, q7_empty = 0;
	cperf_demand demand;
	host_reply finish_ts;
	int i, n;
	int j, req_len;
	//int ret;
	//int counter = 0;
	//sleep(4);
	while(1){
		printf("\n HEyaasss");
		fflush(stdout);
		TIME_SLOT++;
	
		memset(tor_link_capacity, 0, sizeof(tor_link_capacity[0][0]) * 8 * 16);
		memset(curr_timeslot_vlanid, 0, sizeof(curr_timeslot_vlanid[0][0]) * 17 * 4);
		memset(pod_vlanid, 0, sizeof(pod_vlanid[0][0]) * 4 * 4);
		memset(torswitch_port_capacity, 0, sizeof(torswitch_port_capacity[0][0]) * 8 * 2);
		for(j = 0; j < 8; j++){
			for(i = 0; i< 2; i++)
				torswitch_port_capacity[j][i] = 0;
		}
		memset(coreswitch_port_capacity, 0, sizeof(coreswitch_port_capacity[0][0]) * 4 * 4);
	//	memset(prev_vlanid, 0, sizeof(prev_vlanid[0][0]) * 17 * 17);
		
		reply_index = 0;
		memset(reply, 0, sizeof(struct host_reply) * 240);
		

		
		if(request_5000.len == 0 && request_6000.len==0 && request_7000.len == 0){
			pthread_mutex_lock(&no_req_mutex);
			printf("\n Process Demands thread going to sleep");
			fflush(stdout);
			pthread_cond_wait(&no_req_cond, &no_req_mutex);
			printf("\n Process Demands thread waking from sleep");
			fflush(stdout);
			pthread_mutex_unlock(&no_req_mutex);
		}
		
		short dest_index, src_index;
/*
		// sort the request arrays	
		pthread_mutex_lock(&request_7000.lock);
		req_len = request_7000.len;
		pthread_mutex_unlock(&request_7000.lock);	
		if(req_len > 0)
			sort_req_arr(&request_7000, req_len);	

		pthread_mutex_lock(&request_5000.lock);
		req_len = request_5000.len;
		pthread_mutex_unlock(&request_5000.lock);	
		if(req_len > 0)
			sort_req_arr(&request_5000, req_len);	
		
		pthread_mutex_lock(&request_6000.lock);
		req_len = request_6000.len;
		pthread_mutex_unlock(&request_6000.lock);	
		if(req_len > 0)
			sort_req_arr(&request_6000, req_len);	
*/	

	//print_array(request_7000, req_len);
	
		// First Priority (request_7000) - Latency Sensitive!
		
		pthread_mutex_lock(&request_7000.lock);
		req_len = request_7000.len;
		pthread_mutex_unlock(&request_7000.lock);	
		if(req_len > 0)
			sort_req_arr(&request_7000, req_len);	
		
		if(req_len > 0)
		{
			for(i=0; (i < req_len); i++){
				demand = request_7000.request[i];	
				if(demand.flow > 0){
					get_src_dest_index(demand.src, demand.dest, &src_index, &dest_index);
					if(tor_link_capacity[dest_index][demand.dest -1] < 200 && tor_link_capacity[src_index][demand.src - 1] < 200){
						//printf("\n GOING TO FIND PATH");
						find_path(i, 7);
						if(request_7000.request[i].flow == 0){
							pthread_mutex_lock(&request_7000.lock);
							req_len = request_7000.len;
							adjust_req_arr(&request_7000, i, req_len);
							sort_req_arr(&request_7000, req_len-1);	
							pthread_mutex_unlock(&request_7000.lock);
						}
					}
				}
			}
		}
		

			
		// Second Priority (request_5000) - Latency Sensitive!
		pthread_mutex_lock(&request_5000.lock);
		req_len = request_5000.len;
		pthread_mutex_unlock(&request_5000.lock);	
		if(req_len > 0)
			sort_req_arr(&request_5000, req_len);	
		
		if(req_len > 0)
		{
			for(i=0; (i < req_len); i++){
				demand = request_5000.request[i];	
				if(demand.flow > 0){
					get_src_dest_index(demand.src, demand.dest, &src_index, &dest_index);
					if(tor_link_capacity[dest_index][demand.dest -1] < 200 && tor_link_capacity[src_index][demand.src - 1] < 200){
						//printf("\n GOING TO FIND PATH");
						find_path(i, 5);
						if(request_5000.request[i].flow == 0){
							pthread_mutex_lock(&request_5000.lock);
							req_len = request_5000.len;
							adjust_req_arr(&request_5000, i, req_len);
							sort_req_arr(&request_5000, req_len-1);	
							pthread_mutex_unlock(&request_5000.lock);
						}
					}
				}
			}
		}

		
		// Third Priority (request_6000) - Latency Sensitive!
		pthread_mutex_lock(&request_6000.lock);
		req_len = request_6000.len;
		pthread_mutex_unlock(&request_6000.lock);	
		if(req_len > 0)
			sort_req_arr(&request_6000, req_len);	
		
		if(req_len > 0)
		{
			for(i=0; (i < req_len); i++){
				demand = request_6000.request[i];	
				if(demand.flow > 0){
					get_src_dest_index(demand.src, demand.dest, &src_index, &dest_index);
					if(tor_link_capacity[dest_index][demand.dest -1] < 200 && tor_link_capacity[src_index][demand.src - 1] < 200){
						//printf("\n GOING TO FIND PATH");
						find_path(i, 6);
						if(request_6000.request[i].flow == 0){
							pthread_mutex_lock(&request_6000.lock);
							req_len = request_6000.len;
							adjust_req_arr(&request_6000, i, req_len);
							sort_req_arr(&request_6000, req_len-1);	
							pthread_mutex_unlock(&request_6000.lock);
						}
					}
				}
			}
		}


		// SEND REPLY's to HOSTS for this TIMESLOT
		if(reply_index > 0){
		//	pthread_mutex_lock(&ts_complete_lock);
			for(i=0;i<reply_index;i++){
				if(reply[i].port/1000 == 5)
					which_flow = which_flow | (1<<0);
				else if(reply[i].port/1000 == 6)
					which_flow = which_flow | (1<<1);
				else if(reply[i].port/1000 == 7)
					which_flow = which_flow | (1<<2);
				n = write(hostsockfd[reply[i].src],(void *)&reply[i], sizeof(host_reply));	
			//	ts_complete = ts_complete | 1 << (reply[i].src - 1);
			}
		/*	for(i=0;i<16;i++){
				if(ts_complete & (1 << i)){
					finish_ts.timeslot = TIME_SLOT;
					finish_ts.src = i+1;
					finish_ts.dest = 100;
					n = write(hostsockfd[i+1],(void *)&finish_ts, sizeof(host_reply));	
				}
			}
			pthread_mutex_lock(&ts_lock);
			printf("\n &&&&&& TIMESTAMP Process Demands thread going to sleep");
			fflush(stdout);
			pthread_mutex_unlock(&ts_complete_lock);
			pthread_cond_wait(&ts_finished, &ts_lock);
			printf("\n &&&&&& TIMESTAMP Process Demands thread waking from sleep");
			fflush(stdout);
			pthread_mutex_unlock(&ts_lock);*/
			usleep(1500);
		}
		ts_complete = 0;
		
		/*if(which_flow != 4 && which_flow != 2)
			usleep(1500);
		which_flow = 0;*/
	
	/*
		i = 1;
		for(i = 1; i<=16; i++){
			while(host_reqQ_7000[i].qlen != 0){
				ret = find_path(i, 7);
				if(ret < 0) break;
			}
		}
		for(i = 1; i<=16; i++){
			//printf("\n HERE");
			while(host_reqQ_5000[i].qlen != 0){
				printf("\n *************TIME SLOT");
				fflush(stdout);
				ret = find_path(i, 5);
				if(ret < 0) break;
				counter ++;
			}
		}
		for(i = 1; i<=16; i++){
			while(host_reqQ_6000[i].qlen != 0){
				ret = find_path(i, 6);
				if(ret < 0) break;
			}
		}
		//printf("\n COUNTER : %d", counter);
		for(i = 1; i<=16; i++){
			if(host_reqQ_5000[i].qlen != 0) q5_empty |= 1 << (i-1);
			if(host_reqQ_6000[i].qlen != 0) q6_empty |= 1 << (i-1);
			if(host_reqQ_7000[i].qlen != 0) q7_empty |= 1 << (i-1);
		}
	//	send_demands_to_host();
	//	if(response_queue.qlen != 0)
	//		pthread_cond_signal(&res_que_notempty);	
		if(!q5_empty && !q6_empty && !q7_empty){
			printf("\n@@@@@@@@@@ Process Thread sleeping");
			fflush(stdout);
			pthread_mutex_lock(&req_qlock);
			pthread_cond_wait(&queue_not_empty, &req_qlock);
			pthread_mutex_unlock(&req_qlock);
			printf("\n@@@@@@@@@@ Process Thread waking up !!!");
		}
	*/
//		fprintf(send_dmd_fp, "\n *********************************************************************************************************************************");
	//	printf("\n------------------------------>>> QUEUE LENGTH : %d", response_queue.qlen);
		//sleep(2);
		//fflush(stdout);
	}
	printf("\n OUT!!!!!");

}




