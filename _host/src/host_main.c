#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <host.h>
#include <pthread.h>
#include <queue.h>
#include <ifaddrs.h>

pthread_t senddemands_thread, recievedemands_thread, sendpackets_thread;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
pthread_mutex_t req_qlock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t response_qlock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
FILE *traffic_file;
short host_number;
FILE *send_dmd_fp, *recv_dmd_fp, *send_pkts_fp, *transmit_pkts_fp;

//-----RAW SOCKETS-----
int socketfd[16];
int interfaceindex[16];
//--------------------
// socket through which host and arbiter are connected.
//int sockfd ;
int connect_with_arbiter();
int disconnest_with_arbiter();

void open_log_files();
void close_log_files();

void
connect_to_hosts(){
	int i = 0;
	int flags;
	for(i = 0; i<16; i++){	
		if(i+1 != host_number){
			socketfd[i] = open_socket(0, &interfaceindex[i]);
  			//flags = fcntl(socketfd[i], F_GETFL, 0);
  			//fcntl(socketfd[i], F_SETFL, O_NONBLOCK | flags);
		}
	}
	printf("\n HOST %d connections: ",host_number);
	for(i = 0; i<16; i++){	
		printf(" %d", socketfd[i]);
	}
}

void
set_host_number(){
	struct ifaddrs *ifap, *ifa;
    	struct sockaddr_in *sa;
   	char *addr;
	char *ret;
    	char *iface = "-eth0";
	int got_ip = 0;
    	getifaddrs (&ifap);
    	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        	if (ifa->ifa_addr->sa_family==AF_INET) {
            		sa = (struct sockaddr_in *) ifa->ifa_addr;
            		addr = inet_ntoa(sa->sin_addr);
            		char *a = ifa->ifa_name;
            		ret = (char *)strstr(a, iface);
            		if(ret != NULL){
                		if(strncmp(ret, iface, 0)==0){
                     			printf("Interface: %s\tAddress: %s\n", ifa->ifa_name, addr);
					got_ip = 1;
					break;
				}
            		}
       	 	}
    	}
	
	char ip[3];
     	if(got_ip){
                int i=0, j=0, count = 0;
                char ch;      
                while((ch=addr[i])!='\0'){
                        if(count == 3){
                                ip[j] = addr[i];
                                j++;
                        }
                        if(ch == '.'){
                                count++;
                        }
                        i++;
                }
                ip[j] = '\0';
        }
        //printf("\n HOST NUMBER : %d    %s", atoi(ip), ip);
	host_number = atoi(ip);
    	freeifaddrs(ifap);
   	return 0;
}


int
main(int argc, char *argv[]){

	char *file_name = argv[1];
	char *temp = argv[1];
	void **value_ptr = NULL;
	char *token = NULL;
	/* Traffic file - Demand source */
	traffic_file = fopen(file_name, "r");
	if(traffic_file == NULL){
		printf("\n Bad Traffic file. Cannot be opened");
		printf("\n Host is stopping ..");
		return -1;
	}
	printf("\n FILE OPENED !! %s", file_name);

	// Find the host number.
	//char temp[] = argv[1];
	/*int len = strlen(temp);
	if(len == 14){
		memmove(temp, temp+8, 6);
		temp[6] = '\0';	
	}
	else{
		memmove(temp, temp+8, 5);
		temp[5]  = '\0';
	}

	token = strtok(temp, ".");
	memmove(token, token+1, strlen(token));
	host_number = atoi(token); */
	set_host_number();
	printf("\n LOLLLLLZZZ host_number : %d\n",host_number);

	open_log_files();

	/* Establish a connection with Arbiter */
	int sockfd = connect_with_arbiter();
	if(sockfd < 0){
		printf("\n Host is stopping ..");
		return -1;
	}
	
	// Connect with other hosts (raw socket connection)
	connect_to_hosts();


	init_demand_queue();

	/* Three Threads for sending demands, recieving vlan id's+path and sending packets to destination host respectively*/
	pthread_create(&senddemands_thread, 0, (void *)&command_parser, &sockfd);	
	pthread_create(&recievedemands_thread, 0, (void *)&recieve_demands, &sockfd);
	pthread_create(&sendpackets_thread, 0, (void *)&send_packets, &sockfd);
	pthread_join (senddemands_thread, value_ptr);
	pthread_join (recievedemands_thread, value_ptr);
	
	/* Disconnect with Arbiter */
	int retval = disconnest_with_arbiter(sockfd); 
	if(retval < 0)
		printf("\n Failed to disconnect from Arbiter");
	
	//pthread_join (sendpackets_thread, value_ptr);
	fclose(traffic_file);
	close_log_files();
	return 1;

}
int
connect_with_arbiter(){
	int sockfd;
	int portno = 5000;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	int optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
	if (sockfd < 0) {
		printf("\n Cannot open socket at host");
		return -1;
	}
	//server = gethostbyname("localhost");
	server = gethostbyname("20.0.0.100");
	if (server == NULL) {
		printf("\n No server(arbiter) found at 20.0.0.100");
		return -1;
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
		printf("\n Error in connecting with arbiter");
		return -1;
	}
	printf("\n => Connected with Arbiter");
	//fcntl(sockfd, F_SETFL, O_NONBLOCK);
	return sockfd;
}

int
disconnest_with_arbiter(int sockfd){
	int retval = close(sockfd);
	printf("\n => Disconnected with Arbiter");
	return retval;
}

void
open_log_files(){
	
	char hostnm[50];
        hostnm[0]='l';hostnm[1] = 'o';hostnm[2]='g';hostnm[3] = '/';hostnm[4]='h';
        sprintf(hostnm+5, "%d", host_number);
        if(host_number > 9){
                hostnm[7] = '-';
                hostnm[8] = '\0';
	}
        else{
                hostnm[6] = '-';
                hostnm[7] = '\0';
	}
	char filepath[50];
        
	strcpy(filepath, hostnm);
	strcat(filepath, "sent-demands.log");	
	send_dmd_fp = fopen(filepath, "w");
	if(send_dmd_fp == NULL)
		printf("\n Cannot Create arbiter-accept-connections log file");
        
	strcpy(filepath, hostnm);
        strcat(filepath, "recieved-demands.log");	
	recv_dmd_fp = fopen(filepath, "w");
	if(recv_dmd_fp == NULL)
		printf("\n Cannot Create arbiter-recieve-demands log file");
        
	strcpy(filepath, hostnm);
        strcat(filepath, "sent-packets.log");	
	send_pkts_fp = fopen(filepath, "w");
	if(send_pkts_fp == NULL)
		printf("\n Cannot Create arbiter-send-demands log file");	
	
	strcpy(filepath, hostnm);
        strcat(filepath, "transmit-packets.log");	
	transmit_pkts_fp = fopen(filepath, "w");
	if(transmit_pkts_fp == NULL)
		printf("\n Cannot Create arbiteir-send-demands log file");	
}
void
close_log_files(){
	fclose(send_dmd_fp);
	fclose(recv_dmd_fp);
	fclose(send_pkts_fp);
}

/*
int send_to_server(int destn, int cli_portno, int packets)
{
    //int sockfd;
    int n;
    int portno = 51718;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting -----");
    packet p;
    int count = (int)packets / 1000;
    int remaining_packets = packets - count * 1000;
    p.flow = 1000;
    //p.portno = cli_portno;
    p.dest = destn;
    int i;
    while(count > 0){
    	 n = write(sockfd,(void *)&p,sizeof(packet));
   	 if (n < 0) 
        		 error("ERROR writing to socket");
    	count--;
   }
    if(remaining_packets != 0){
    	 p.flow = remaining_packets;
    	 n = write(sockfd,(void *)&p,sizeof(packet));
   	 if (n < 0) 
        		 error("ERROR writing to socket");
    }
    return 0;
    close(sockfd);
}

*/



