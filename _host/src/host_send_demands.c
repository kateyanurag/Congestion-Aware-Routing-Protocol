
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

int
send_demands(int destn, int packets, int port, int *sockfd){
/*	host_demand demand;
	int n;
	int count = (int)packets / 50;
	printf("\n Loop Count : %d", count);	
    	int remaining_packets = 0;
    	demand.flow = 50;
    	demand.dest = (short)destn;
    	demand.vlanid = 0;
	demand.port = (short)port;
    	demand.src = host_number;
    	//printf("\nSending to Arbiter - Src: %hd   Dest : %d, Packets: %d   Port: %d n",host_number, destn, demand.flow, port);
    	while(count > 0){
    		printf("\nSending to Arbiter - Src: %hd   Dest : %d, Packets: %d   Port: %d n",host_number, destn, demand.flow, port);
    		fprintf(send_dmd_fp, "Sending to Arbiter - Src: %hd   Dest : %d, Flow: %d  Port: %d\n",host_number, destn, demand.flow, port);
    		n = write(*sockfd,(void *)&demand, sizeof(host_demand));
   		if (n < 0) {
   			printf("\n Cannot send demamd to Arbiter");
   			return -1;
   		} 	
    		count--;
   	}
    	if(remaining_packets != 0){
    	 	demand.flow = remaining_packets;
		printf("\n Writing Remaining ...............");
    	 	n = write(*sockfd,(void *)&demand, sizeof(host_demand));
   	 	if (n < 0) {
   			printf("\n Cannot send demamd to Arbiter");
   			return -1;
   		}
    	}
	fflush(send_dmd_fp);
    	return 0; */

	cperf_demand demand;	
	int n;
    	demand.flow = packets/50; // A flow has 5 packets
    	demand.dest = (short)destn;
	demand.port = (short)port;
    	demand.src = host_number;
    	demand.ack = 0;
    	n = write(*sockfd,(void *)&demand, sizeof(host_demand));
   	if (n < 0) {
   		printf("\n Cannot send demamd to Arbiter");
   		return -1;
  	} 	
    	fprintf(send_dmd_fp, "Sending to Arbiter - Src: %hd   Dest : %d, Flow: %d  Port: %d  Ack: %d\n",host_number, demand.dest, demand.flow, demand.port, demand.ack);
	fflush(send_dmd_fp);
}	

void
command_parser(int *sockfd){
	ssize_t read;
	size_t length = 0, len = 0;
	char *line = NULL;
	char *token;
	short count = 0;
	int bytes;
	char demand_unit;
	//char *prev_token;
	int destn=0, packets=0;
	int port=0; 
	while((read = getline(&line, &length, traffic_file)) != -1){
		fflush(stdout);
		len = strlen(line);
		if(line[len - 1] == '\n'){
			demand_unit = line[len - 3];
		//		demand_unit = line[len - 2];
			line[len - 2] = '\0';
		}
		else{
			demand_unit = line[len - 2];
		//	demand_unit = line[len - 1];
			line[len - 1] = '\0';
		}
		printf("\n LINE : %s      DEMAND UNIT :-%c-", line, demand_unit);
		token = strtok(line, " ");
		count = 0;
		while(token != NULL){
			count ++;
			/*if(strcmp(prev_token, "-d")){
				memmove(token, token+1, strlen(token));
				destn = atoi(token);
			}if(strcmp(prev_token, "-p")){
				port = atoi(token);
			}if(strcmp(prev_token, "-n")){
				bytes = atoi(token);
				printf("\n BYTES: %d", bytes);
				packets = (int)(bytes * pow(10, 6));
				if(packets % 1024 == 0)
					packets = packets / 1024;
				else
					packets = packets / 1024 + 1;
			}*/
			
			if(count == 2){
				 memmove(token, token+1, strlen(token));
				 destn = atoi(token);
			}
			if(count == 4)
				port = atoi(token);
			if(count == 6){
				//printf("\n Token Bytes: %d", token);
				bytes = atoi(token);
				//printf("\n BYTES: %d", bytes);
				if(demand_unit == 'K')
					packets = (int)(bytes * pow(10, 3));
				else if(demand_unit == 'M')
					packets = (int)(bytes * pow(10, 6));
				else if(demand_unit == 'G')
					packets = (int)(bytes * pow(10, 9));
				
				if(packets % 1000 == 0) // 1000 bytes: size of each packet
					packets = packets / 1000;
				else
					packets = packets / 1000 + 1;
			}
			//prev_token = token;
			token = (char *)strtok(NULL, " ");
		}
		//	packets = 2009;
			if(send_demands(destn, packets, port, sockfd) < 0){
				printf("\n send_demands() failed !");
				printf("\n senddemands_thread exiting ....");
				return;
			}
	//	sleep(0.005);
	}
	printf("\n SEND DEMANDS THREAD RETURNED\n");

}
