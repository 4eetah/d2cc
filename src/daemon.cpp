#include <stdio.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define D2CC_SOCK_PATH /tmp/d2ccd.sock

void handle_incoming_request(int fd, short event, void* args)
{
	perror("Got to handle incoming request\n");
}

static void listen_unix_socket()
{
	int listenfd, n;
	char buf[4096];
	struct sockaddr_un servaddr;
	
	listenfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (listenfd < 0) {
		perror("unix socket create error\n");
		return -1;
	}
	unlink(D2CC_SOCK_PATH);
	
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, D2CC_SOCK_PATH, sizeof(addr)-1);
	if (bind(listenfd, (struct sockaddr*)servaddr, sizeof(servaddr)) < 0) {
		perror("unix socket bind error\n");
		return -1;
	}
	if (listen(listenfd, 0) < 0) {
		perror("unix socket listen error\n");
		return -1;
	}
	
	while ((n = read(listenfd, buf, sizeof(buf))) > 0) {
		write(1, buf, n);
	}
	if (n == 0) {
		perror("EOF on unix socket\n");
		return -1;
	} else {
		perror("unix socket error\n");
		return -1;
	}
	return 0;
}

void listen_inet_socket()
{
	
}

void remote_listener()
{
	
}

int main(int argc, char** argv)
{
	
}
