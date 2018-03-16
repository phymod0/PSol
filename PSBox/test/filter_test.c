


/*

	DESCRIPTION:
		Intercept DNS packets from an interface.

	TODO:
		- Parse URL strings from DNS packets

	FIXME:
		- None

	XXX:
		- None

*/



#ifndef FILTER_TEST_C
#define FILTER_TEST_C



#include <stdio.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

#include "filter_test.h"
#include "dns.h"



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

		udp_hdr = (struct udphdr*)(pkt+sizeof(struct iphdr));

		if (udp_hdr->dest == htons(53)) {
			printf("Found a DNS packet!\n");
			
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

	handle = nfq_open();
	if (handle == NULL) {
		perror("Main handle initialization failed");
		return 1;
	}

	res = nfq_bind_pf(handle, AF_INET);
	if (res < 0) {
		perror("Main handle binding failed");
		nfq_close(handle);
		return 1;
	}

	q_handle = nfq_create_queue(handle, 0, &logger_cb, NULL);
	if (q_handle == NULL) {
		perror("Queue handle initialization failed");
		nfq_close(handle);
		return 1;
	}

	res = nfq_set_mode(q_handle, NFQNL_COPY_PACKET, 0xffff);
	if (res < 0) {
		perror("Failed to set queue handle mode");
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

	nfq_destroy_queue(q_handle);
	nfq_close(handle);

	return 0;

}



#endif /* FILTER_TEST_C */



