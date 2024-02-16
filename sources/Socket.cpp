#include "Socket.h"

int create_fd() {
    int fd_ = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0);
    if (fd_ < 0) {
        perror("socket failed!");
        exit(-1);
    }
    return fd_;
}

Socket::Socket(int fd): fd_(fd){

}
Socket::~Socket(){
    close(fd_);
}

int Socket::fd() const{
    return fd_;
}

void Socket::set_reuseaddr(bool on){
    int flag = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &flag, static_cast<socklen_t>(sizeof(flag)));
}
void Socket::set_reuseport(bool on){

}
void Socket::set_tcpnodelay(bool on){
    int flag = on ? 1 : 0; // 用bool不行
    setsockopt(fd_, SOL_SOCKET, TCP_NODELAY, &flag, static_cast<socklen_t>(sizeof(flag)));
}
void Socket::set_keepalive(bool on){

}
void Socket::set_sndbuff(int size){
    int send_buf_size = size;
    setsockopt(fd_, SOL_SOCKET, SO_SNDBUF, &send_buf_size, sizeof(send_buf_size));
}

void Socket::bind(const InetAddr& server_addr){
    if (::bind(fd_, server_addr.addr(), sizeof(sockaddr)) < 0) {
        perror("bind failed!");
        close(fd_); // 失败就关闭监听fd
        exit(-1);
    }
}
void Socket::listen(int max_num){
    if (::listen(fd_, max_num) != 0) {
        perror("listen failed!");
        close(fd_); // 失败就关闭监听fd
        exit(-1);
    }
}
int Socket::accept(InetAddr& client_addr){
    // 
    struct sockaddr_in ret_addr;
    socklen_t len = sizeof(sockaddr_in);
    int cfd = ::accept4(fd_, (sockaddr*)&ret_addr, &len, SOCK_NONBLOCK);
    // int cfd = ::accept(fd_, (sockaddr*)&ret_addr, &len);
    // int flag = fcntl(cfd, F_GETFL, 0);
    // flag |= O_NONBLOCK;
    // fcntl(cfd, F_SETFL, flag);
    // InetAddr client_addr(ret_addr);
    client_addr = InetAddr(ret_addr);
    return cfd;
}