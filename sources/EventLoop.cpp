#include "EventLoop.h"
EventLoop::EventLoop():epoll_tree_(new Epoll) {


}

void EventLoop::run() {
    while (true) {
        std::vector<Channel*> channels = epoll_tree_->loop_ch();
        for (auto &channel : channels) {
            channel->event_handle();     
        }
    }
}

Epoll* EventLoop::ep() {
    return epoll_tree_;
}

EventLoop::~EventLoop() {

    delete epoll_tree_;
}
