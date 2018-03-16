


/*

	DESCRIPTION:
		- Intercept DNS packets from an interface

	TODO:
		- Test MAC and time functions
		- Test server communication

	FIXME:
		- None so far

	XXX:
		- In get_macaddr() and flush_queue()

*/



#ifndef FILTER_C
#define FILTER_C



#define DUMMY				0
#define QUEUE_MAXLIMIT		16
#define IP_MAXBUFLEN		16
#define TIME_MAXBUFLEN		32
#define MACADDR_MAXBUFLEN	32
#define IFACE_NAME			"wlan0"



// Standard
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Packet reading tools
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include "dns.h"

// Netfilter
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

// Default
#include "result_queue.h"
#include "comm.h"



// Main queue containing the logged data
result_queue rqueue;



static void ip2str(unsigned char *saddr, char *ipbuf) {

	snprintf(ipbuf, IP_MAXBUFLEN, "%u.%u.%u.%u", saddr[0], saddr[1], saddr[2], saddr[3]);

	return;

}

/*
	Takes a pointer to a NULL-terminated string
	representing the target IP address in it's
	ASC II form (IP) and the pointer to the char
	array to save the hardware address analogous
	to this IP address in the kernel's ARP table
*/
static void get_macaddr(char *saddr, char *HW) {

	/*

		XXX:
			This may be outdated in the newer OpenWRT distros
			Re-visit this after the prototype is complete
			Find a more portable way to grab the mac address

	*/

	int iplen=0;
	char buf[256];
	char IP[IP_MAXBUFLEN];

	char B1[17]={'0','0',':','0','0',':','0','0',':','0','0',':','0','0',':','0','0'};
	char B2[17]={'f','f',':','f','f',':','f','f',':','f','f',':','f','f',':','f','f'};
	char B3[17]={'F','F',':','F','F',':','F','F',':','F','F',':','F','F',':','F','F'};

	ip2str(saddr, IP);

	memcpy(HW,B2,17);

	while (IP[iplen]) ++iplen;

	FILE *fd;
	fd=fopen("/proc/net/arp","r");

	fgets(buf,256,fd);

	while (fgets(buf,256,fd)) {

		// WARNING
		// offset for buf and other values
		// must be re-evaluated from
		// the new ARP table structure

		if (memcmp(buf,IP,iplen)) continue;

		if (!memcmp(B1,buf+41,17)) continue;
		if (!memcmp(B2,buf+41,17)) continue;
		if (!memcmp(B3,buf+41,17)) continue;

		memcpy(HW,buf+41,17);

		break;

	}

	fclose(fd);

	return;

}

static void get_time(char *timebuf) {

	time_t rawtime;
	struct tm *timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(timebuf, TIME_MAXBUFLEN, "%c", timeinfo);

	return;

}

static void flush_queue() {

	// Make contact with server here and move all elements from the queue to the server

	char *queue_str = result_queue_str(&rqueue);

	if (comm_send_data(queue_str, strlen(queue_str)) < 0) {
		printf("Server communication failure...\n");
		free(queue_str);
		return;
	}

	free(queue_str);

	// result_queue_print(&rqueue);
	result_queue_destroy(&rqueue);

	return;

}

static int logger_cb(
	struct nfq_q_handle *q_handle,
	struct nfgenmsg *nf_msg,
	struct nfq_data *data,
	void *cb_data
) {

	struct nfqnl_msg_packet_hdr *nl_hdr;
	int id, pkt_sz;
	unsigned char *pkt;
	struct iphdr *ip_hdr;

	nl_hdr = nfq_get_msg_packet_hdr(data);

	if (nl_hdr) {
		id = ntohl(nl_hdr->packet_id);
	} else {
		id = 0;
		printf("Warning: nfq_get_msg_packet_hdr returned NULL\n");
	}

	pkt_sz = nfq_get_payload(data, &pkt);

	ip_hdr = (struct iphdr*)pkt;

	// printf("Received IP packet with protocol id %d...\n", ip_hdr->protocol);

	if (ip_hdr->protocol == IPPROTO_UDP) {

		struct udphdr *udp_hdr;

		// printf("Found a UDP packet!\n");

		udp_hdr = (struct udphdr*)(pkt + sizeof(struct iphdr));

		if (udp_hdr->dest == htons(53)) {

			int i;
			char macaddr[MACADDR_MAXBUFLEN], current_time[TIME_MAXBUFLEN];

			printf("Found a DNS packet!\n");

			query_list qlist = extract_urls(pkt + sizeof(struct iphdr) + sizeof(struct udphdr));

			get_macaddr((unsigned char*)&(ip_hdr->saddr), macaddr);
			get_time(current_time);

			for (i = 0; i < qlist.n_queries; i++)
				result_queue_push(&rqueue, macaddr, qlist.urls[i], current_time);

			if (rqueue.n_results > QUEUE_MAXLIMIT)
				flush_queue();

			// print_urls(qlist);
			free_urls(qlist);

			// return nfq_set_verdict(q_handle, id, NF_DROP, pkt_sz, pkt);

		}

		// printf("%d -> %d\n", ntohs(udp_hdr->source), ntohs(udp_hdr->dest));

	}

	return nfq_set_verdict(q_handle, id, NF_ACCEPT, pkt_sz, pkt);

}

int main() {

	int res, fd;
	char buf[4096];
	struct nfq_handle *handle;
	struct nfq_q_handle *q_handle;
	struct nfnl_handle *nl_handle;

	rqueue = result_queue_create();

	handle = nfq_open();
	if (handle == NULL) {
		perror("Main handle initialization failed");
		result_queue_destroy(&rqueue);
		return 1;
	}

	res = nfq_bind_pf(handle, AF_INET);
	if (res < 0) {
		perror("Main handle binding failed");
		result_queue_destroy(&rqueue);
		nfq_close(handle);
		return 1;
	}

	q_handle = nfq_create_queue(handle, 0, &logger_cb, NULL);
	if (q_handle == NULL) {
		perror("Queue handle initialization failed");
		result_queue_destroy(&rqueue);
		nfq_close(handle);
		return 1;
	}

	res = nfq_set_mode(q_handle, NFQNL_COPY_PACKET, 0xffff);
	if (res < 0) {
		perror("Failed to set queue handle mode");
		result_queue_destroy(&rqueue);
		nfq_destroy_queue(q_handle);
		nfq_close(handle);
		return 1;
	}

	nl_handle = nfq_nfnlh(handle);
	fd = nfnl_fd(nl_handle);

	while (1) {

		int recv_len;

		recv_len = recv(fd, buf, sizeof(buf), 0);

		if (recv_len < 0) {
			printf("recv() returned %d, exiting...\n", recv_len);
			break;
		}

		nfq_handle_packet(handle, buf, recv_len);

	}

	result_queue_destroy(&rqueue);
	nfq_destroy_queue(q_handle);
	nfq_close(handle);

	return 0;

}



#endif /* FILTER_C */



