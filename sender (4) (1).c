#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>

#define MY_DEST_MAC0	0x11
#define MY_DEST_MAC1	0x12
#define MY_DEST_MAC2	0x13
#define MY_DEST_MAC3	0x14
#define MY_DEST_MAC4	0x15
#define MY_DEST_MAC5	0x16

int sfd;
struct ifreq if_idx;
struct ifreq if_mac;
int tx_len = 0;
char sendbuf[1024];
char input[100];

struct ether_header *eh;
struct iphdr *iph;

struct sockaddr_ll socket_address;
char ifName[IFNAMSIZ];

void create_sockaddr(){
	socket_address.sll_ifindex = if_idx.ifr_ifindex;
	socket_address.sll_halen = ETH_ALEN;
	socket_address.sll_addr[0] = MY_DEST_MAC0;
	socket_address.sll_addr[1] = MY_DEST_MAC1;
	socket_address.sll_addr[2] = MY_DEST_MAC2;
	socket_address.sll_addr[3] = MY_DEST_MAC3;
	socket_address.sll_addr[4] = MY_DEST_MAC4;
	socket_address.sll_addr[5] = MY_DEST_MAC5;
}

void create_eth_header(){
    memset(sendbuf, 0, 1024);
    eh = (struct ether_header *) sendbuf;
    iph = (struct iphdr *) (sendbuf + sizeof(struct ether_header));

    eh->ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
	eh->ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
	eh->ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
	eh->ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
	eh->ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
	eh->ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
	
    eh->ether_dhost[0] = MY_DEST_MAC0;
	eh->ether_dhost[1] = MY_DEST_MAC1;
	eh->ether_dhost[2] = MY_DEST_MAC2;
	eh->ether_dhost[3] = MY_DEST_MAC3;
	eh->ether_dhost[4] = MY_DEST_MAC4;
	eh->ether_dhost[5] = MY_DEST_MAC5;

    eh->ether_type = htons(ETH_P_IP);

    tx_len += sizeof(struct ether_header);

    printf("Enter payload: ");
    scanf("%s", input);
    for(int i=0;i<sizeof(input);i++){
        sendbuf[tx_len++] = input[i];
    }

    printf("Ethernet header made successfully.  \n");
}

void create_raw_socket(){
    if ((sfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) < 0) {
	    perror("Socket not created.");
	}
    
}

int main(int argc, char *argv[])
{
	if (argc > 1){
		strcpy(ifName, argv[1]);
        printf("Interface name is %s.  \n", ifName);
    }
	else{
        printf("Usage: sudo ./filename [Interface name].\n");
        exit(1);
    }

    create_raw_socket();

	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sfd, SIOCGIFINDEX, &if_idx) < 0){
        perror("Interface not found. \n");
    }

	memset(&if_mac, 0, sizeof(struct ifreq));
	strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sfd, SIOCGIFHWADDR, &if_mac) < 0){
	    perror("Interface MAC address not found. \n");
    }

    create_eth_header();
    create_sockaddr();

	if (sendto(sfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(socket_address)) < 0){
        printf("Sending packet failed.\n");
    }
    else{
        printf("Packet sent successfully.  \n");
    }
	    
	return 0;
}
