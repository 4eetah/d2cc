#include "daemon.h"
#include <sys/select.h>
#include <string.h>
#include <errno.h>

#define errMsg(e)  do { perror(e); } while(0)
#define errExit(e) do { perror(e); exit(1); } while(0)

#define D2CC_SOCK "/tmp/d2cc_local.sock"
#define SERV_PORT 9877

#define max(a,b)    ((a) > (b) ? (a) : (b))

Daemon::Daemon(const std::string& unix_path)
{
}

Daemon::~Daemon()
{
}

void Daemon::run() 
{
    // all scheduling done here for now
    int slotsAvailable = std::thread::hardware_concurrency();

    int i, maxi, maxfd, listenfd, connfd, sockfd;
    int nready, client[FD_SETSIZE];
    ssize_t n;
    fd_set rset, allset;
    char buf[4096];
    socklen_t clilen;
    struct sockaddr_in remote_cliaddr, servaddr;
    struct sockaddr_un local_cliaddr;

    remote_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (remote_listenfd < 0)
        errExit("remote_listenfd socket\n");

    local_listenfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (local_listenfd < 0)
        errExit("local_listenfd socket\n");

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    ret = bind(remote_listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    if (ret < 0)
        errExit("remote_listendfd bind");
    if (listen(remote_listenfd, 5) < 0)
        errExit("remote_listenfd listen");

    unlink(D2CC_SOCK);
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sun_family = AF_UNIX;
    strncpy(servaddr.sun_path, D2CC_SOCK, sizeof(servaddr)-1);

    ret = bind(local_listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    if (ret < 0)
        errExit("local_listenfd bind");
    if (listen(local_listenfd, 5) < 0)
        errExit("remote_listenfd listen");

    maxfd = max(local_listenfd, remote_listenfd);
    maxi  = -1;
    for (i = 0; i < FD_SETSIZE; ++i)
        client[i] = -1;
    FD_ZERO(&allset);
    FD_SET(remote_listenfd, &allset);
    FD_SET(local_listenfd, &allset);

    char bufntop[1024];
    /*
     * main loop
     */
    for ( ; ; ) {
        rset = allset;
        nready = select(maxfd+1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(remote_listenfd, &rset)) {
            clilen = sizeof(cliaddr);
            connfd = accept(remote_listenfd, (struct sockaddr*) &cliaddr, &clilen);
            if (connfd < 0)
                errExit("remote_listenfd accept");
#ifdef DEBUG
            printf("new remote client: %s, port %d\n",
                inet_ntop(AF_INET, &cliaddr.sin_addr, bufntop, sizeof(bufntop)),
                ntohs(cliaddr.sin_port));
#endif
            for (i = 0; i < FD_SETSIZE; ++i)
                if (client[i] < 0) {
                    client[i] = connfd;
                    break;
                }
            if (i == FD_SETSIZE)
                errExit("too many client");

            FD_SET(connfd, &allset);
            if (connfd > maxfd)
                maxfd = connfd;
            if (i > maxi)
                maxi = i;

            if (--nready <= 0)
                continue;
        }

        if (FD_ISSET(local_listenfd, &rset)) {
            clilen = sizeof(cliaddr);
            connfd = accept(local_listenfd, (struct sockfd*) &cliaddr, &clilen);
            if (connfd < 0)
                errExit("local_listenfd accept");

#ifdef DEBUG
            printf("new remote client: %s, port %d\n",
                inet_ntop(AF_INET, &cliaddr.sin_addr, bufntop, sizeof(bufntop)),
                ntohs(cliaddr.sin_port));
#endif
            for (i = 0; i < FD_SETSIZE; ++i)
                if (client[i] < 0) {
                    client[i] = connfd;
                    break;
                }
            if (i == FD_SETSIZE)
                errExit("too many client");

            FD_SET(connfd, &allset);
            if (connfd > maxfd)
                maxfd = connfd;
            if (i > maxi)
                maxi = i;

            if (--nready <= 0)
                continue;
        }

        for (i = 0; i <= maxi; ++i) {
            if ((sockfd = client[i]) < 0)
                continue;
            if (FD_ISSET(sockfd, &rset)) {
                if ((n = read(sockfd, buf, 4096)) == 0) {
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[-1] = -1;
                } else {
                    write(sockfd, buf, n);
                }

                if (--nready <= 0)
                    break;
            }
        }
    }
}
