/***
 *
 * Generate/Receive Ethernet Packet
 *
 * Coded by Yasusi Kanada
 * Ver 1.0  2012-5-20	Initial version
 *
 ***/

#define VLAN		Yes
#define DEBUG		1
#define SRC_PORT 24341
#define DST_PORT 5000
#define DEST_IP "10.0.0.2"

#include <Ether.h>
#include <fcntl.h>
 #include <arpa/inet.h>


#define MAX_PACKET_SIZE	2048
	// Sufficiently larger than the MTU

 
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
int32_t open_socket(int32_t index, int32_t *rifindex, int hostnumber) {
  unsigned char buf[MAX_PACKET_SIZE];
  int32_t i;
  int32_t ifindex;
  struct ifreq ifr;
  struct sockaddr_ll sll;
  char ifname[IFNAMSIZ];

char str[4];
str[0] = 'h';
sprintf((char *)str + 1, "%d", hostnumber);
if(hostnumber > 9)
	str[3] = '\0';
else
	str[2] = '\0';

  char interface_name []= {'-', 'e', 't', 'h', '0'};
  strcat((char*)str, (char *)interface_name);
  printf("\n ------------------ Interface Name : %s   IFNAMSIZ: %d\n", str, IFNAMSIZ);
  strncpy(ifname, str, IFNAMSIZ);
  ifname[strlen(ifname) - 1] = '0' + index;

  int32_t fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (fd == -1) {	
    printf("%s - ", ifname);
    perror("socket");
    _exit(1);
  };

  // get interface index
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
  if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1) {
    printf("%s - ", ifname);
    perror("SIOCGIFINDEX");
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
  sll.sll_protocol = htons(ETH_P_ALL);
  sll.sll_ifindex = ifindex;
  
/**************************************************************/
  sll.sll_halen = ETH_ALEN; /* Assume ethernet -- addr_len=6! */
  memcpy(&sll.sll_addr, &ifr.ifr_hwaddr.sa_data, sll.sll_halen);
/**************************************************************/             
  printf("***************Hardware Address : %s***********\n\n",sll.sll_addr);

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
		     uint32_t src, uint32_t dest, uint32_t port, char *payload) {

	if(DEBUG) 	printf("\n payload length=%d        SRC:%d   Dest:%d   Port: %d\n",(int)strlen(payload), src, dest, port);

//ssize_t packetSize = sizeof(EtherPacket);	
  ssize_t packetSize =  18 + 20 + 8 + strlen(payload);
  //packet->destMAC1 = htons(destMAC1);
  //packet->destMAC2 = htonl(destMAC2);
  packet->srcMAC1 = htons(srcMAC1);
  packet->srcMAC2 = htonl(srcMAC2);
#ifdef VLAN
  packet->VLANTag = htonl(vlanTag);
#endif
  packet->type = htons(type);
	


	/***********************************************************************/
	if (DEBUG) printf("After adding ethernet_header: packetSize=%d\n\n",(int)packetSize);

	//if (DEBUG) printf("iphdrSize=%d, udphdrSize=%d\n\n", (int) sizeof(struct iphdr), (int) sizeof(struct udphdr) );
	
	packet->iph_ihl = 5;
	packet->iph_ver = 4;
	packet->iph_tos = 16; // Low delay
	packet->iph_ident = htons(54321);
	packet->iph_ttl = 64; // hops
	packet->iph_protocol = 17; // UDP
	// Source IP address, can be spoofed 
	//iph->saddr = inet_addr(inet_ntoa(((struct sockaddr_in *)&if_ip.ifr_addr)->sin_addr));
	 packet->iph_sourceip = inet_addr("192.168.0.112");
	 
	 
	// Destination IP address 
	packet->iph_destip = inet_addr("10.0.0.16");
	//packetSize += sizeof(struct iphdr *);
	
	//struct iphdr *iph = (struct iphdr *) (payload + sizeof(EtherPacket));
		/* IP Header */
	/*packet->iph->iph_ihl = 5;
	packet->iph->iph_ver = 4;
	packet->iph->iph_tos = 16; // Low delay
	packet->iph->iph_ident = htons(54321);
	packet->iph->iph_ttl = 64; // hops
	packet->iph->iph_protocol = 17; // UDP
	// Source IP address, can be spoofed 
	//iph->saddr = inet_addr(inet_ntoa(((struct sockaddr_in *)&if_ip.ifr_addr)->sin_addr));
	 packet->iph->iph_sourceip = inet_addr("192.168.0.112");
	// Destination IP address 
	packet->iph->iph_destip = inet_addr(DEST_IP);
	//packetSize += sizeof(struct iphdr *);
	*/
	if (DEBUG) printf("After adding ip_header: packetSize=%d\n",(int)packetSize);
	
	//struct udphdr *udph = (struct udphdr *) (payload + sizeof(struct iphdr) + sizeof(EtherPacket));
	/* UDP Header */

	packet->udph_srcport = htons(SRC_PORT);
	//packet->udph_destport = htons(DST_PORT);
	packet->udph_destport = htons(port);
	packet->udph_chksum = 0; // skip
	if (DEBUG) printf("After adding udp_header: packetSize=%d\n",(int)packetSize);
	
	//packetSize+= sizeof(struct udph *);
	
	/***********************************************************************/

  //packetSize+=strlen(payload);

  	/* Length of UDP payload and header */
	packet->udph_len = htons(8);//htons(sizeof(struct udphdr *));//htons(packetSize - sizeof(EtherPacket) - sizeof(struct iphdr));
	/* Length of IP payload and header */
	packet->iph_len = htons(20);//htons(sizeof(struct iphdr *));//htons(packetSize - sizeof(EtherPacket));
	/* Calculate IP checksum on completed header */
	packet->iph_chksum = csum( (unsigned short *)payload, 20 + 8 );
	printf("packet->iph->iph_chksum=%d\n", packet->iph_chksum);

  //packet->payload = htonl(payload);
  strncpy(packet->payload, payload, packetSize);
  printf("packet->payload=%s--->payload=%s\n",packet->payload,payload );
  return packetSize;
}


int32_t lastPayload = -1;

/**
 * Print IPEC packet content
 */
void printPacket(EtherPacket *packet, ssize_t packetSize, char *message) {
	/*
#ifdef VLAN
  printf("%s #%d (VLAN %d) from %08x%04x to %08x%04x\n", message, ntohl(packet->payload), ntohl(packet->VLANTag) & 0xFFF,
#else
  printf("%s #%d from %08x%04x to %08x%04x\n", message, ntohl(packet->payload),
#endif
	 ntohs(packet->srcMAC1), ntohl(packet->srcMAC2),
	 ntohs(packet->destMAC1), ntohl(packet->destMAC2));
  lastPayload = ntohl(packet->payload); */
}


/**
 * Send packets to terminals
 */
int sendPackets(int32_t fd, int32_t ifindex, uint16_t SrcMAC1, uint32_t SrcMAC2,
		 uint16_t DestMAC1, uint32_t DestMAC2, uint16_t type, uint32_t vlanTag, short src, short dest, short port) {
	  //int32_t i;
	  char packet[MAX_PACKET_SIZE];
	  char payload[1000];
	int i = 0;
	for(i = 0; i < 1000; i++)
		payload[i] = '0'; 
	payload[999]='\0';
	 // memset(payload, 0, 1000);
	  //++(	*count);
	  struct sockaddr_ll sll;
	  memset(&sll, 0, sizeof(sll));
	  sll.sll_family = AF_PACKET;
	  sll.sll_protocol = htons(ETH_P_ALL);	// Ethernet type = Trans. Ether Bridging
	  sll.sll_ifindex = ifindex;

	  ssize_t packetSize = createPacket((EtherPacket*)packet, DestMAC1, DestMAC2, SrcMAC1, SrcMAC2, type, vlanTag, src, dest, port, payload);
	  ssize_t sizeout = sendto(fd, packet, packetSize, 0, (struct sockaddr *)&sll, sizeof(sll));
	  if (sizeout < 0) {
	    	printf("\n Failed to send packets : sendto error !!! ");
	    	return -1;
	  } else {
	    	if (DEBUG) {
	      		printf("%d bytes sent through interface (ifindex) %d\n", (int32_t)sizeout, (int32_t)ifindex);
	    	}
	 }
	 printPacket((EtherPacket*)packet, packetSize, "Sent:    ");
	 return sizeout;
}


void transmit_packets(int32_t fd, int32_t ifindex, uint16_t SrcMAC1, uint32_t SrcMAC2,
		 uint16_t DestMAC1, uint32_t DestMAC2, uint16_t type, uint16_t vlanID, short src, short dest, short port, short flow) {
 // unsigned char buf[MAX_PACKET_SIZE];
 // int32_t sendCount = 0;
 // int32_t receiveCount = 0;
//  time_t lastTime ;//= time(NULL);
//  int32_t replyDelay = 0;
//  int32_t i;
  uint32_t vlanTag = 0x81000000 | vlanID;
  int cnt = 0;
	

	flow = 2;
  for(cnt = 0; cnt < (int)flow; cnt++)
  {
  	int retval = sendPackets(fd, ifindex, SrcMAC1, SrcMAC2, DestMAC1, DestMAC2, type, vlanTag, src, dest, port);
  	if(retval < 0){
  		printf("\n transmit_packets problem\n");
  	}
  }

  // Sending and receiving packets:
 /* for (; sendCount < MaxCommCount && receiveCount < MaxCommCount;) 
  {
    if (DestMAC2 != 0 && replyDelay <= 0) {
      int32_t currTime = time(NULL);
      if (currTime - lastTime >= Period) {
	if (DEBUG) printf("currTime=%d lastTime=%d\n", currTime, (int32_t)lastTime);
	int retval = sendPackets(fd, ifindex, SrcMAC1, SrcMAC2, DestMAC1, DestMAC2, type, vlanTag);
	lastTime = currTime;
      }
    }
    ssize_t sizein = recv(fd, buf, MAX_PACKET_SIZE, 0);
    if (sizein >= 0) {
      EtherPacket *packet = (EtherPacket*) buf;
      if (DestMAC2 == 0) {
	DestMAC1 = ntohs(packet->srcMAC1);
	DestMAC2 = ntohl(packet->srcMAC2);
	replyDelay = InitialReplyDelay;
      } else if (replyDelay > 0) {
	replyDelay--;
      }
      printPacket(packet, sizein, "Received:");
      receiveCount++;
    } else {
      usleep(10000); // sleep for 10 ms
    }*/ 
}


/**
 * Main program
 */
//int32_t main(int32_t argc, char **argv) {
/*
int 
trans_packets(int32_t flow, int32_t, int32_t port, int32_t vlanid){
  int32_t ifindex;
  int32_t myTermNum = 0;
  int32_t destTermNum = 0;
  int32_t ifnum = 0;
  int32_t interface_num = 0;
  uint16_t vlanID = 1;
  int32_t i;

  myTermNum = 0;
  destTermNum = 1;
  //interface_num = atoi(argv[3]);
  //vlanid = atoi(argv[4]);


  // Get terminal and interface numbers from the command line:
  int32_t count = 0;
  if (++count < argc) {
    myTermNum = atoi(argv[count]);	// My terminal number
  }
  if (myTermNum >= NTerminals || myTermNum < 0) {
    printf("My terminal number (%d) too large\n", myTermNum);
    myTermNum = 0;
  }

  if (++count < argc) {
    destTermNum = atoi(argv[count]);	// Destination terminal number
  }
  if (destTermNum >= NTerminals || destTermNum < 0) {
    printf("Destination terminal number (%d) too large\n", destTermNum);
    destTermNum = 1;
  }

  if (++count < argc) {
    ifnum = atoi(argv[count]);		// Interface number
  }

  if (++count < argc) {
    vlanID = atoi(argv[count]);		// VLAN ID
  }
  if (vlanID < 1 || 4095 < vlanID) {
    printf("VLAN ID %d out of range (1..4095)\n", vlanID);
    vlanID = 1;
  }

  

  // Set locators and IDs using terminal number:
  uint16_t SrcMAC1  = MAC1[myTermNum];
  uint32_t SrcMAC2  = MAC2[myTermNum];
  uint16_t DestMAC1 = MAC1[destTermNum];
  uint32_t DestMAC2 = MAC2[destTermNum]; 

  uint16_t SrcMAC1  = 0x0001;
  uint32_t SrcMAC2  = 0x00000002;
  uint16_t DestMAC1 = 0x0003;
  uint32_t DestMAC2 = 0x0000004;

  printf("eth%d terminal#=%d VLAN:%d srcMAC:%08x%04x destMAC:%08x%04x\n",
	 ifnum, myTermNum, vlanID,
	 (int32_t)SrcMAC1, (int32_t)SrcMAC2, (int32_t)DestMAC1, (int32_t)DestMAC2);

  
  int32_t fd = open_socket(ifnum, &ifindex);

  // Set non-blocking mode:
  int32_t flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, O_NONBLOCK | flags);

  transmit_packets(fd, ifindex, SrcMAC1, SrcMAC2, DestMAC1, DestMAC2, ETH_P_Exp, vlanID);
}
*/
