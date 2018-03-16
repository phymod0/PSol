
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

int main() {

	int sock_fd;
	struct sockaddr_in server_addr, client_addr;
	socklen_t addrlen;
	char send_buf[] = "Hello world!";

	sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(12049);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	client_addr.sin_family = AF_INET;
	client_addr.sin_port = 0; // Any address available
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	addrlen = sizeof(struct sockaddr_in);

	if (bind(sock_fd, (struct sockaddr*)&client_addr, addrlen) < 0) {
		perror("Bind");
		return -1;
	}

	if (connect(sock_fd, (struct sockaddr*)&server_addr, addrlen) < 0) {
		perror("Connect");
		return -1;
	}

	if (send(sock_fd, send_buf, sizeof(send_buf), 0) <= 0) {
		perror("Send");
		return -1;
	}

	printf("Fuck...\n");

	printf("Sent %s...\n", send_buf);

	close(sock_fd);

	return 0;

}
