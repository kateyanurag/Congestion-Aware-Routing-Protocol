#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h> 
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <queue.h>
#include <arbiter.h>

short gethost_number(char *ipaddress){
	//printf("\n HOST IP : %s", ipaddress);
	char *token = NULL;
	token = strtok(ipaddress, ".");
	int count = 1;
	while(token != NULL){
		if(count == 4){
			//printf("\n ---------- Token : %s", token);
			return atoi(token);
		}
		token = (char *)strtok(NULL, ".");
		count ++;
	}
	return 0;
}
void
accept_connections(short *sockfd){
    
    	//int *sockfd;
	socklen_t clilen, len;
	struct sockaddr_storage addr;
	struct sockaddr_in cli_addr;
	char ipstr[INET6_ADDRSTRLEN];
	int port = 0;
	int i = 0;
	clilen = sizeof(cli_addr);
	short newsockfd = -1;
	short hostnum;
	printf("\n ************* ACCEPT CONNECTIONS THREAD :    SOCKFD: %d", *sockfd);
	while(i <= 16){
	//	printf("\n IN n %d   Socket:%d", i, *sockfd);
		fflush(stdout);
		//fprintf(accept_con_fp, "\n Before ------");
		newsockfd = (short)accept(*sockfd, (struct sockaddr *) &cli_addr, &clilen);
		//fprintf(accept_con_fp, "\n After -----");
		fflush(stdout);
		//printf("\n Here");
		if(newsockfd > 0){
			//hostsockfd[1] = newsockfd;
			len = sizeof addr;
			getpeername(newsockfd , (struct sockaddr *)&addr, &len);
			struct sockaddr_in *s = (struct sockaddr_in *)&addr;
			port = ntohs(s->sin_port);
			inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
			//printf("\n CLIENTS IP: %s", ipstr);
			char *ipaddress = ipstr;
		//	printf("\n IP ADDR : %s", ipstr);
			hostnum = gethost_number(ipaddress);
			//printf("\n  hostsockfd[%hd] = %hd", hostnum, newsockfd);
			
			if(newsockfd > biggestfd)
				biggestfd = newsockfd;
			// Lock
		//	printf("\n +++ Biggest FD : %d", biggestfd);
		//	pthread_mutex_lock(&sockfd_lock);
			hostsockfd[hostnum] = newsockfd;
		//	printf("\n *** New Sockfd Connection = %d", hostsockfd[hostnum]);
		//	pthread_mutex_unlock(&sockfd_lock);
			
		//	FD_SET(newsockfd, &hostsockfdset);
	
			//fcntl(newsockfd, F_SETFL, O_NONBLOCK);
			//char *ipaddr = ipstr;
			//printf("\n Host IP address: %s      HOST Number: %d ", ipstr, gethost_number(ipstr));
			//printf("      Peer port      : %d", port);
			fprintf(accept_con_fp, "\nConnection from  h%hd     IP: %s    Socketfd: %hd   Port:%d", hostnum, ipstr, newsockfd, port);
			printf("\nConnection from  h%hd     IP: %s    Socketfd: %hd   Port:%d", hostnum, ipstr, newsockfd, port);
			fflush(accept_con_fp);
		}
		i = (i+1)%16;
	}
	
}
