#include "wrap.h"
#include <string>
#include <thread>

class Daemon
{
    int listenfd_unix;

public:

    Daemon(const std::string& unix_path = D2CC_SOCK);
    ~Daemon();

    void run();
};
