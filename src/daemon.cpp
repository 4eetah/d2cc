#include "daemon.h"

Daemon::Daemon(const std::string& unix_path)
{
    // initialize unix socket for listening to the
    // local request from toolchain
    struct sockaddr_un servaddr;
    listenfd_unix = Sock::Socket(AF_UNIX, SOCK_STREAM, 0);
    unlink(unix_path.c_str());
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
    strncpy(servaddr.sun_path, unix_path.c_str(), sizeof(servaddr)-1);

    Sock::Bind(listenfd_unix, (SA*)&servaddr, sizeof(servaddr));
    Sock::Listen(listenfd_unix, LISTENQ);
}

Daemon::~Daemon()
{
    Unix::Close(listenfd_unix);
}

void Daemon::run() 
{
    // all scheduling done here for now
    int slotsAvailable = std::thread::hardware_concurrency();

    pid_t pid = Unix::Fork(); 

    if (pid == 0) {
        int connfd, n;
        char buf[4096];
        connfd = Sock::Accept(listenfd_unix, NULL, NULL);

        while ((n = Unix::Read(connfd, buf, sizeof(buf))) > 0)
            Unix::Write(1, buf, n);

        if (n < 0) {
            err_sys("Unix socket error\n");
        }

    } else {
    }
}
