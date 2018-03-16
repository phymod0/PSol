


/*

	DESCRIPTION:
		Enter a description

	TODO:
		Modify the existing code into an event-driven system!
		We don't the queue handler to be too slow...

*/



#ifndef COMM_C
#define COMM_C



#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "comm.h"



#define SUCCESS			0
#define FAILURE			-1

// Network config
#define CLIENT_IFNAME	"eth0"
#define SERVER_IP		"127.0.0.1"
#define SERVER_PORT		2048



static int fill_server_addr(struct sockaddr_in *server_addr) {

	server_addr->sin_family			= AF_INET;
	server_addr->sin_port			= htons(SERVER_PORT);
	server_addr->sin_addr.s_addr	= inet_addr(SERVER_IP);

	return SUCCESS;

}

static int fill_client_addr(struct sockaddr_in *client_addr) {

	int fd;
	struct ifreq if_req;

	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if_req.ifr_addr.sa_family = AF_INET;
	strcpy(if_req.ifr_name, CLIENT_IFNAME);

	ioctl(fd, SIOCGIFADDR, &if_req);

	close(fd);

	client_addr->sin_family	= AF_INET;
	client_addr->sin_port	= 0;
	client_addr->sin_addr	= ((struct sockaddr_in*)&(if_req.ifr_addr))->sin_addr;

	return SUCCESS;

}

/*

	@buf:
		Buffer containing the data to send
	@len:
		Length of the data

	Send len bytes from character array buf to the server and return success status

*/
int comm_send_data(char *buf, int len) {

	int sock_fd;
	char *buf_ptr;
	struct sockaddr_in client_addr, server_addr;
	socklen_t addr_len;

	fill_server_addr(&server_addr);
	fill_client_addr(&client_addr);

	addr_len = sizeof(struct sockaddr_in);

	sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (bind(sock_fd, (struct sockaddr*)&client_addr, addr_len) < 0) {
		perror("comm_send_data() -> bind()");
		close(sock_fd);
		return FAILURE;
	}

	if (connect(sock_fd, (struct sockaddr*)&server_addr, addr_len) < 0) {
		perror("comm_send_data() -> connect()");
		close(sock_fd);
		return FAILURE;
	}

	while (len) {

		int res = send(sock_fd, buf, len, 0);

		if (res < 0) {
			perror("comm_send_data() -> send()");
			close(sock_fd);
			return FAILURE;
		}

		buf += res;
		len -= res;

	}

	close(sock_fd);

	return SUCCESS;

}



#endif /* COMM_C */



