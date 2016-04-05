#include "wrap.h"
#include <thread>

// daemon is also a scheduler for now
class Daemon
{
    int listenfd_unix;
    int cpu;

public:

    Daemon(const std::string& unix_path = D2CC_SOCK) 
    {
        // initialize unix socket for listening to the
        // local request from toolchain
        struct sockaddr_un servaddr;
        listenfd_unix = Sock::Socket(AF_UNIX, SOCK_STREAM, 0);
        unlink(unix_path);
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sun_family = AF_UNIX;
        strncpy(servaddr.sun_path, unix_path, sizeof(servaddr)-1);
        Sock::Bind(listenfd_unix, (SA*)&servaddr, sizeof(servaddr));
        Sock::Listen(listenfd_unix, LISTENQ);

        cpu = std::thread::hardware_concurrency();
    }

    ~Daemon()
    {
        Unix::Close(listenfd_unix);
    }

    void run() 
    {
        pid_t pid = Unix::Fork(); 

        if (pid == 0) {
            int connfd, n;
            char buf[4096];
            connfd = Sock::Accept(listenfd, NULL, NULL);
            
            // some reasonable scheduling here

            while ((n = Unix::Read(connfd, buf, sizeof(buf))) > 0)
                Unix::Write(1, buf, n);
            if (n < 0) {
                err_sys("Unix socket error\n");
                return -1;
            }

        } else {
        }
    }

};
