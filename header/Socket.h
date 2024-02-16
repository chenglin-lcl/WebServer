#ifndef _SOCKET_H
#define _SOCKET_H
#include "InetAddr.h"
#include <unistd.h>
#include <netinet/tcp.h>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
// 用于封装与socket fd相关的代码
class Socket {
public:
    Socket(int fd);
    ~Socket();

    int fd() const;
    
    void set_reuseaddr(bool on);
    void set_reuseport(bool on);
    void set_tcpnodelay(bool on);
    void set_keepalive(bool on);
    void set_sndbuff(int size);

    void bind(const InetAddr& server_addr);
    void listen(int max_num=128);
    int accept(InetAddr& client_addr);

private:
    const int fd_; // 不需要修改fd的值
};
int create_fd();
#endif