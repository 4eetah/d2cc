#include "wrap.h"

enum Request
{
    LOCAL;
    REMOTE;
};

enum Reply
{
    RUN_REMOTELY;
    RUN_LOCALLY;    
};

class Scheduler
{
    int cpu;
    int listen_sched;
        
public:
    Scheduler(std::string sched_path = SCHED_SOCK) {
        cpu = std::thread::hardware_concurrency();
        
        // init socket for listening for job requests
        struct sockaddr_un servaddr;
        listenfd_unix = Sock::Socket(AF_UNIX, SOCK_STREAM, 0);
        unlink(sched_path);
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sun_family = AF_UNIX;
        strncpy(servaddr.sun_path, sched_path, sizeof(servaddr)-1);
        Sock::Bind(listenfd_unix, (SA*)&servaddr, sizeof(servaddr));
        Sock::Listen(listenfd_unix, LISTENQ);
    }
    ~Scheduler() {};

    void run() {
        int connfd = Sock::Accept(listen_sched, NULL, NULL);
    }
}
