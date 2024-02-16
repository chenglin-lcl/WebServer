#ifndef _EVVENTLOOP_H
#define _EVVENTLOOP_H
#include "Channel.h"
#include "Epoll.h"

class EventLoop {
public:
    EventLoop();
    ~EventLoop();
    void run();
    Epoll* ep();

private:
    Epoll* epoll_tree_;

};
#endif