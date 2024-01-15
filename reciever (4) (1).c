#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>

#define DEST_MAC0	0x11
#define DEST_MAC1	0x12
#define DEST_MAC2	0x13
#define DEST_MAC3	0x14
#define DEST_MAC4	0x15
#define DEST_MAC5	0x16

#define ETHER_TYPE	0x0800

char sender[INET6_ADDRSTRLEN];
int sfd, ret, i;
int sockopt;
ssize_t numbytes;
struct ifreq ifopts;
struct ifreq if_ip;
struct sockaddr_storage their_addr;
uint8_t buf[1024];
char ifName[IFNAMSIZ];

struct ether_header *eh = (struct ether_header *) buf;
struct iphdr *iph = (struct iphdr *) (buf + sizeof(struct ether_header));
struct udphdr *udph = (struct udphdr *) (buf + sizeof(struct iphdr) + sizeof(struct ether_header));

void create_raw_socket(){
    if ((sfd = socket(PF_PACKET, SOCK_RAW, htons(ETHER_TYPE))) == -1) {
		perror("Socket not created.");
	}

    strncpy(ifopts.ifr_name, ifName, IFNAMSIZ-1);
	ioctl(sfd, SIOCGIFFLAGS, &ifopts);
	ifopts.ifr_flags |= IFF_PROMISC;
	ioctl(sfd, SIOCSIFFLAGS, &ifopts);
    
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
		printf("Cannot set socket to be reused.");
		close(sfd);
		exit(1);
	}

    if (setsockopt(sfd, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1)	{
		printf("Cannot bind socket to device.");
		close(sfd);
		exit(1);
	}

    printf("Raw socket created successfully.  \n");
}

int main(int argc, char *argv[])
{
	if (argc > 1){
		strcpy(ifName, argv[1]);
    }
	else{
        printf("Usage: sudo ./filename [Interface name].\n");
        exit(1);
    }

	create_raw_socket();

	memset(&if_ip, 0, sizeof(struct ifreq));

    printf("Listening for ethernet packets.\n");
    while(1){
        numbytes = recvfrom(sfd, buf, 1024, 0, NULL, NULL);
        if(sizeof(buf) > 0){
            if (eh->ether_dhost[0] == DEST_MAC0 &&
                eh->ether_dhost[1] == DEST_MAC1 &&
                eh->ether_dhost[2] == DEST_MAC2 &&
                eh->ether_dhost[3] == DEST_MAC3 &&
                eh->ether_dhost[4] == DEST_MAC4 &&
                eh->ether_dhost[5] == DEST_MAC5   ) {
                    printf("Ethernet packet recieved. Destination MAC address matches.\n");
                    
                    int start = sizeof(struct ether_header);
                    int end = numbytes - (sizeof(struct iphdr) + sizeof(struct udphdr));
                    char output[100];
                    bzero(output, 100);
                    for (i=start; i<end; i++){
                        output[i-start] = (char)buf[i];
                    }
                    if(strcmp(output, "exit")==0){
                        printf("Closing listener.\n");
                        break;
                    }
                    printf("Payload recieved: %s.\n", output);
                    
                printf("Listening for ethernet packets.\n");
            } 
            else {
                continue;
            }  
        }
    }
	close(sfd);
	return 0;
}
