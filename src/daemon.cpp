#include "wrap.h"

int listenUnixSocket();
int listenInetSocket();

int main(int argc, char** argv)
{
    listenUnixSocket();	
}

int listenUnixSocket()
{
	int listenfd, connfd, n;
	char buf[4096];
	struct sockaddr_un servaddr;
    
    listenfd = Sock::Socket(AF_UNIX, SOCK_STREAM, 0);

    unlink(D2CC_SOCK);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
    strncpy(servaddr.sun_path, D2CC_SOCK, sizeof(servaddr)-1);

    Sock::Bind(listenfd, (SA*)&servaddr, sizeof(servaddr));
    Sock::Listen(listenfd, LISTENQ);
    connfd = Sock::Accept(listenfd, NULL, NULL);
    
    while ((n = Unix::Read(connfd, buf, sizeof(buf))) > 0)
        Unix::Write(1, buf, n);

    if (n < 0) {
        err_sys("Unix socket error\n");
        return -1;
    }
    
    return 0;
}

int listenInetSocket()
{
    return 0;
}
