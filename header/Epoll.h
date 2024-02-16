#ifndef _EPOLL_H
#define _EPOLL_H
#include <sys/epoll.h>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include "Channel.h"
static const int MAXEVENT = 100;
class Channel;
class Epoll {
public:
    Epoll();
    ~Epoll();

    void add(int fd, uint32_t events);
    void del(int fd);

    // Epoll中的添加fd，都改为添加Channel
    void modify_channel(Channel* ch);
    void del_channel(Channel* ch);
    std::vector<epoll_event> loop(int timeout=-1);
    std::vector<Channel*> loop_ch(int timeout=-1);

private:
    
    int epoll_fd_;
    epoll_event evs_[MAXEVENT];
};
#endif
