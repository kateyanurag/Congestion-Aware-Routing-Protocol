/***
 *
 * Software-based Ethernet Common Header
 *
 * Coded by Yasusi Kanada
 * Ver 1.0  2012-5-20   Initial version
 *
 ***/
#ifndef __ETHER_H__
 #define __ETHER_H__
#include <linux/if_packet.h>
#include <linux/if_ether.h> 
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <elf.h>
#include <string.h>


// 0       6      12      12/16 14/18           18/22
// +-------+-------+---------+----+---------------+
// | DMAC  | SMAC  |8100 VLAN|Type|Payload (4Bfix)|
// +-------+-------+---------+----+---------------+
//                  <-------> when VLAN == Yes



struct _EtherHeader {
  uint16_t destMAC1;
  uint32_t destMAC2;
  uint16_t srcMAC1;
  uint32_t srcMAC2;
#ifdef VLAN
  uint32_t VLANTag;
#endif
  uint16_t type;
//Ehternet header 18 bytes

/********************************************************/
// Can create separate header file (.h) for all headers' structure
// The IP header's structure
	unsigned char      iph_ihl:4, iph_ver:4;		//1 Byte
 	unsigned char      iph_tos;								//1 Byte
	unsigned short int iph_len;								//1 Byte
	unsigned short int iph_ident;							//2 Bytes
	unsigned short int iph_offset;							//2 Byte
	unsigned char      iph_ttl;								//1 Byte
	unsigned char      iph_protocol;						//1 Byte
	unsigned short int iph_chksum;							//2 Bytes
	unsigned int       iph_sourceip;						//4 Bytes
	unsigned int       iph_destip;							//4 Bytes
// total ip header length: 20 bytes (=160 bits)

// UDP header's structure
 unsigned short int udph_srcport;						//2 Bytes
 unsigned short int udph_destport;					//2 Bytes
 unsigned short int udph_len;								//2 Bytes
 unsigned short int udph_chksum;						//2 Bytes
// total udp header length: 8 bytes (=64 bits)


  
  char payload[1024];
} __attribute__((packed));

typedef struct _EtherHeader EtherPacket;

#define ETH_P_Exp	0x88b5
extern int32_t open_socket(int32_t index, int32_t *rifindex, int host_number);
extern void transmit_packets(int32_t fd, int32_t ifindex, uint16_t SrcMAC1, uint32_t SrcMAC2, uint16_t DestMAC1, uint32_t DestMAC2, uint16_t type, uint16_t vlanID, short src, short dest, short port, short flow);

/*2 of 3 items
README.txtEther.hraw_socket.cDisplaying README.txt.*/
#endif
