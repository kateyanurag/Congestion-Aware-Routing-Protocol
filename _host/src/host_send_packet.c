/*int
send_packets(int *sockfd){
	return 1;
}
*/
#include <Ether.h>
#include <queue.h>
#include <unistd.h>
#include <fcntl.h>
#include <host.h>

static int curr_ts = 0;
static int prev_dest = 0;
int
send_packets(int *sockfd){
	demand_qnode_data demand;
	cperf_demand ack_packet;
	int32_t interface_index;
	int interface_num = 0;
	//int sockfd = open_socket(interface_num, &interface_index, (int)host_number);
//	int32_t flags = fcntl(sockfd, F_GETFL, 0);
	//fcntl(sockfd, F_SETFL, O_NONBLOCK | flags);
	uint16_t src_mac1  = 0x0001;
  	uint32_t src_mac2  = 0x00000002;
  	uint16_t dest_mac1 = 0x0000;
  	uint32_t dest_mac2 = 0x000002;
  //	uint16_t packet_type = ETH_P_Exp;
	int32_t retval = 0;
	int n;
	demand.src = 0;
	while(1){
		fflush(stdout);
		dequeue_demand(&demand);
		if(demand.src != 0){
		//	printf("\nPackets Sent: Src: %hd    Dest: %hd   Flow: %hd  vlanid: %hd  Port: %hd", demand.src, demand.dest, demand.flow, demand.vlanid, demand.port);
			fprintf(send_pkts_fp, "Transmitting Flow: Timestamp: %d Src: %hd    Dest: %hd   Flow: %hd  vlanid: %hd  Port: %hd\n",demand.timeslot, demand.src, demand.dest, demand.flow, demand.vlanid, demand.port);
		//	transmit_packets(sockfd, interface_index, src_mac1, src_mac2, dest_mac1, dest_mac2, packet_type, demand.vlanid, demand.src, 
		//	demand.dest, demand.port, demand.flow);
			//sleep(1);
		//	transmit_packets();
			//demand.flow = 20;
			//if(demand.port == 5001)
    	//		sleep((float)rand()/pow(10, 10));
			///if(demand.port != 5001){
				//retval = transmit_packets((int32_t)demand.src, (int32_t)demand.dest, (int32_t)demand.port, (int16_t)demand.vlanid, (int32_t)demand.flow);
				//usleep(5000);
			
			// TS finish message
			if(demand.dest == 100 && curr_ts == demand.timeslot){
				fprintf(send_pkts_fp, "Sending Timeslot ACK: Timeslot: %d \n",curr_ts);
				fflush(send_pkts_fp);
				ack_packet.src = host_number; 
				ack_packet.flow = curr_ts; 
				ack_packet.ack = 1; 
				n = write(*sockfd,(void *)&ack_packet, sizeof(host_reply));	
				curr_ts = 0;	
			}else{
			/*	if(prev_dest == demand.dest)
					usleep(50);*/
				curr_ts = demand.timeslot;
				//if(curr_ts == prev_ts || curr_ts == 0){
				retval = transmit_packets((int32_t)demand.src, (int32_t)demand.dest, (int32_t)demand.port, (int16_t)demand.vlanid, (int32_t)demand.flow);
				prev_dest = demand.dest;
				//usleep(50);
				//prev_ts = curr_ts;
			}	
			//}	
		//	if(demand.src == 5001)
		}else{
			// send Ack to arbiter
		/*	if(curr_ts > 0){
				fprintf(send_pkts_fp, "Sending Timeslot ACK: Timeslot: %d \n",curr_ts);
				fflush(send_pkts_fp);
				ack_packet.src = host_number; 
				ack_packet.flow = curr_ts; 
				ack_packet.ack = 1; 
				n = write(*sockfd,(void *)&ack_packet, sizeof(host_reply));	
				curr_ts = 0;
			}*/
			pthread_mutex_lock(&req_qlock);
		//	printf("\n>>>>>>>>> Going In MUTEX QUEUE");
			pthread_cond_wait(&queue_not_empty, &req_qlock);
		//	printf("\n<<<<<<<<<<<COming Out of  MUTEX QUEUE");
			pthread_mutex_unlock(&req_qlock);
		}	
		demand.src = 0;
		fflush(send_pkts_fp);
	}
	return 1;
}
