
#define VLAN		Yes
#define DEBUG		1
#define SRC_PORT 24341
#define DST_PORT 5000


#include <netinet/in.h>
#include <fcntl.h>
#include <math.h>
#include <Ether.h>
#include <host.h>

#define MAX_PACKET_SIZE	2048
 
// Global Variables src, dest, port, flow.
int32_t src;
int32_t dest;
int32_t port;
int32_t flow;
char ip_dest[20];
char *DEST_IP = "10.0.0.";
char d[3];
int32_t fd;
static int opened = 0;
// Function for checksum calculation. From the RFC,
// the checksum algorithm is:
//  "The checksum field is the 16 bit one's complement of the one's
//  complement sum of all 16 bit words in the header.  For purposes of
//  computing the checksum, the value of the checksum field is zero."
unsigned short csum(unsigned short *buf, int nwords)
{       //
        unsigned long sum;
        for(sum=0; nwords>0; nwords--)
                sum += *buf++;
        sum = (sum >> 16) + (sum &0xffff);
        sum += (sum >> 16);
        return (unsigned short)(~sum);
}


/********************************************************/

#define Period		1

enum commMode {SendAndReceive = 0, ReceiveThenSend = 1};

#define ETH_P_Exp	0x0800
	// Ethernet type = IEEE 802.1 Local Experimental Ethertype 1

#define NTerminals	4
uint16_t MAC1[NTerminals] = {0x0000, 0x0000, 0x0200, 0x0200};
uint32_t MAC2[NTerminals] = {0x00000001, 0x00000002, 0x00000003, 0x00000004};

#define InitialReplyDelay	40
#define MaxCommCount		3

#define IFNAME	"h1-ethX"
	// or "gretapX"


extern void _exit(int32_t);


/**
 * Open a socket for the network interface
 */
int32_t open_socket(int32_t index, int32_t *rifindex) {
  unsigned char buf[MAX_PACKET_SIZE];
  int32_t i;
  int32_t ifindex;
  struct ifreq ifr;
  struct sockaddr_ll sll;
  unsigned char ifname[IFNAMSIZ];
	printf("\n IN OPEN SOCKET ....");
 // strncpy(ifname, IFNAME, IFNAMSIZ);
	  
	char str[9];
	str[0] = 'h';
	
	sprintf((char *)str+1, "%d", host_number);
	if(host_number > 9)
		str[3] = '\0';
	else
		str[2] = '\0';
	//printf("\n #### SOURCE :%d   %s", src, str);

  char interface_name []= {'-', 'e', 't', 'h', '0', '\0'};
  strcat((char*)str, (char *)interface_name);
  //fprintf(transmit_pkts_fp,"\n Host number : %d ------------------ Interface Name : %s   IFNAMSIZ: %d\n", host_number, str, IFNAMSIZ);
  fflush(transmit_pkts_fp);
  strncpy(ifname, str, IFNAMSIZ);
	ifname[strlen(ifname) - 1] = '0' + index;
//ifname[strlen(ifname) - 1] = '0' + index;

  //int32_t fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  int32_t fd = socket(AF_PACKET, SOCK_RAW, htons(IPPROTO_UDP));
  if (fd == -1) {
    printf("%s - ", ifname);
    perror("socket");
    _exit(1);
  };

  // get interface index
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
  if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1) {
    fprintf(transmit_pkts_fp, "%s", ifname);
    fflush(transmit_pkts_fp);
    perror("SIOCGIFINDEX hahahaha");
    _exit(1);
  };
  ifindex = ifr.ifr_ifindex;
  *rifindex = ifindex;

  // set promiscuous mode
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
  printf("ifname = %s\n", ifname);
  
  ioctl(fd, SIOCGIFFLAGS, &ifr);
  ifr.ifr_flags |= IFF_PROMISC;
  ioctl(fd, SIOCSIFFLAGS, &ifr);

/***************************************************************/  
  strncpy((char *)&ifr.ifr_name, ifname, IFNAMSIZ);
  ioctl(fd, SIOCGIFHWADDR, &ifr);
/**************************************************************/             

  memset(&sll, 0xff, sizeof(sll));
  sll.sll_family = AF_PACKET;
  //sll.sll_protocol = htons(ETH_P_ALL);
  sll.sll_protocol = htons(IPPROTO_UDP);
  sll.sll_ifindex = ifindex;
  
/**************************************************************/
  sll.sll_halen = ETH_ALEN; /* Assume ethernet -- addr_len=6! */
  memcpy(&sll.sll_addr, &ifr.ifr_hwaddr.sa_data, sll.sll_halen);
/**************************************************************/             
  printf("***************Hardware Address : %s***********\n\n",sll.sll_addr);

  int optval = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
  
  if (bind(fd, (struct sockaddr *)&sll, sizeof(sll)) == -1) {
    printf("%s - ", ifname);
    perror("bind");
    _exit(1);
  };

  /* flush all received packets. 
   *
   * raw-socket receives packets from all interfaces
   * when the socket is not bound to an interface
   */
  do {
    fd_set fds;
    struct timeval t;
    FD_ZERO(&fds);	
    FD_SET(fd, &fds);
    memset(&t, 0, sizeof(t));
    i = select(FD_SETSIZE, &fds, NULL, NULL, &t);
    if (i > 0) {
      recv(fd, buf, i, 0);
    };
    if (DEBUG) printf("interface %d flushed\n", ifindex);
  } while (i);

  if (DEBUG) printf("%s opened (fd=%d interface=%d)\n", ifname, fd, ifindex);

  return fd;
}


/**
 * Create an IPEC packet
 */
ssize_t createPacket(EtherPacket *packet, uint16_t destMAC1, uint32_t destMAC2,
		     uint16_t srcMAC1, uint32_t srcMAC2, uint16_t type, uint32_t vlanTag,
		     char payload[]) {

//	if(DEBUG) 	printf("payload length=%d\n",(int)strlen(payload));

	ssize_t packetSize = sizeof(EtherPacket) + 1000;	
  //ssize_t packetSize =  18 + 20 + 8 + strlen(payload);
  packet->destMAC1 = htons(destMAC1);
  packet->destMAC2 = htonl(destMAC2);
  packet->srcMAC1 = htons(srcMAC1);
  packet->srcMAC2 = htonl(srcMAC2);
#ifdef VLAN
  packet->VLANTag = htonl(vlanTag);
#endif
  packet->type = htons(type);
	


	/***********************************************************************/
	//if (DEBUG) printf("After adding ethernet_header: packetSize=%d\n\n",(int)packetSize);

	//if (DEBUG) printf("iphdrSize=%d, udphdrSize=%d\n\n", (int) sizeof(struct iphdr), (int) sizeof(struct udphdr) );
	
	packet->iph_ihl = 5;
	packet->iph_ver = 4;
	packet->iph_tos = 16; // Low delay/
	packet->iph_len = htons(20 + 8 +1000);//htons(sizeof(struct iphdr *));//htons(packetSize - sizeof(EtherPacket));
	//packet->iph_ident = htons(54321);
	packet->iph_ttl = 64; // hops
	packet->iph_ident = 0;
	packet->iph_offset = 0;
	packet->iph_protocol = 17; // UDP
	packet->iph_chksum = csum( (unsigned short *)(packet+18), 10);
	// Source IP address, can be spoofed 
	//iph->saddr = inet_addr(inet_ntoa(((struct sockaddr_in *)&if_ip.ifr_addr)->sin_addr));
	
	char ip[10] = {'1', '0', '.', '0', '.', '0', '.'};
	char host[3];
	//int src = 6;
	sprintf(host, "%d", src);
	if(src > 9)
		host[2] = '\0';
	else
		host[1] = '\0';
	strcat(ip, host);	
	//printf("\n SPURCE IP =================== %s", ip); 
	packet->iph_sourceip = inet_addr(ip);
	// Destination IP address 
	//packet->iph_destip = inet_addr(DEST_IP);
	packet->iph_destip = inet_addr(ip_dest);
	//if (DEBUG) printf("After adding ip_header: packetSize=%d\n",(int)packetSize);
	
	/* UDP Header */

	packet->udph_srcport = htons(port);
	packet->udph_destport = htons(port);
	packet->udph_len = htons(8+1000);//htons(sizeof(struct udphdr *));//htons(packetSize - sizeof(EtherPacket) - sizeof(struct iphdr));
	//printf("packet->iph->iph_chksum=%d\n", packet->iph_chksum);
  	return packetSize;
}


int32_t lastPayload = -1;

/**
 * Print IPEC packet content
 */
void printPacket(EtherPacket *packet, ssize_t packetSize, char *message) {
}


/**
 * Send packets to terminals
 */
inline void sendPackets(int32_t fd, int32_t ifindex, uint16_t SrcMAC1, uint32_t SrcMAC2,
		 uint16_t DestMAC1, uint32_t DestMAC2, uint16_t type, uint32_t vlanTag,
		 int32_t *count) {
  int32_t i;
  char packet[MAX_PACKET_SIZE];
  //char payload[15] = "Hello_Computer";
  char payload[1001] = {0};
//	int i;
	for(i = 0; i< 1000; i++)
		payload[i] = '0';
	payload[1000] = '\0';
  ++(*count);
  struct sockaddr_ll sll;
  memset(&sll, 0, sizeof(sll));
  sll.sll_family = AF_PACKET;
  sll.sll_protocol = htons(IPPROTO_UDP);	// Ethernet type = Trans. Ether Bridging
  sll.sll_ifindex = interfaceindex[dest-1];

  ssize_t packetSize = createPacket((EtherPacket*)packet, DestMAC1, DestMAC2,
				    SrcMAC1, SrcMAC2, type, vlanTag, payload);
 // printf("\n Socket: %d", socketfd[dest-1]);
  ssize_t sizeout = sendto(socketfd[dest-1], packet, packetSize, 0, (struct sockaddr *)&sll, sizeof(sll));
 // printPacket((EtherPacket*)packet, packetSize, "Sent:    ");
  if (sizeout < 0) {
    perror("sendto:hahahahah");
  } 
	else {
      printf("%d bytes sent through interface (ifindex) %d   DestIP:%s \n", (int32_t)sizeout, (int32_t)interfaceindex[dest-1], ip_dest);
    /*if (DEBUG) {
      printf("%d bytes sent through interface (ifindex) %d\n",
	     (int32_t)sizeout, (int32_t)ifindex);
    }*/
  }
  fflush(stdout);
}


void sendReceive(int32_t fd, int32_t ifindex, uint16_t SrcMAC1, uint32_t SrcMAC2,
		 uint16_t DestMAC1, uint32_t DestMAC2, uint16_t type, uint16_t vlanID) {
  unsigned char buf[MAX_PACKET_SIZE];
  int32_t sendCount = 0;
  int32_t receiveCount = 0;
  time_t lastTime = time(NULL);
  int32_t replyDelay = 0;
  int32_t i;
  uint32_t vlanTag = 0x81000000 | vlanID;

		
		int32_t j;
		i = 1;
		flow = flow + 50;
		while(i <= flow){
			j = 1;
			while(j <= 10){
				sendPackets(fd, ifindex, SrcMAC1, SrcMAC2, DestMAC1, DestMAC2, type, vlanTag,&sendCount);
				j++;
			}
			usleep(50);
			i = i + 10;
		}

		fprintf(transmit_pkts_fp, "\n Transmitted %d  packets to h%d",flow, dest);
}


/**
 * Main program
 */
//int32_t main(int32_t argc, char **argv) {
inline int32_t transmit_packets(int32_t source, int32_t destination, int32_t portno, int16_t vlan_id, int32_t flow_chunk) {
  int32_t ifindex;
  int32_t myTermNum = 0;
  int32_t destTermNum = 1;
  int32_t ifnum = 0;
  uint16_t vlanID = 1;
  int32_t i;

	src = source;
	dest = destination;
	port = portno;
	flow = flow_chunk;
	vlanID = vlan_id;
//	DEST_IP
	ip_dest[0]='\0';
	sprintf(d, "%d", dest);
	if(dest <= 9)
		d[1] = '\0';
	else
		d[2] = '\0';
	strcat(ip_dest, DEST_IP); 
        strcat(ip_dest, d);
	fprintf(transmit_pkts_fp,"\n DESTIP: ==%s==", ip_dest);

  // Set locators and IDs using terminal number:
  uint16_t SrcMAC1  = MAC1[myTermNum];
  uint32_t SrcMAC2  = MAC2[myTermNum];
  uint16_t DestMAC1 = MAC1[destTermNum];
  uint32_t DestMAC2 = MAC2[destTermNum];
  //printf("eth%d terminal#=%d VLAN:%d srcMAC:%08x%04x destMAC:%08x%04x\n",ifnum, myTermNum, vlanID,(int32_t)SrcMAC1, (int32_t)SrcMAC2, (int32_t)DestMAC1, (int32_t)DestMAC2);
	fflush(stdout);

  sendReceive(socketfd[dest-1], ifindex, SrcMAC1, SrcMAC2, DestMAC1, DestMAC2, ETH_P_Exp, vlanID);
}
