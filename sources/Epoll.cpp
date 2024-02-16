#include "Epoll.h"

Epoll::Epoll() {
    epoll_fd_ = epoll_create(1);
    if (epoll_fd_ == -1) {
        perror("epoll failed!");
        exit(-1);
    }

}
Epoll::~Epoll() {
    close(epoll_fd_);
}

void Epoll::add(int fd, uint32_t events) {
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = events; // 读事件，默认水平触发
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("epoll_ctl add failed!");
        exit(-1);
    }
}
void Epoll::del(int fd) {
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
}

void Epoll::modify_channel(Channel* ch) {
    epoll_event ev;
    ev.data.ptr = ch; // 指向new出来的对象
    ev.events = ch->events(); // 读事件，默认水平触发
    
    if (ch->in_epoll()) { // 在树上就修改
        if ( epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, ch->fd(), &ev) == -1) {
            perror("epoll_ctl mod failed!");
            exit(-1);
        }
    } else { // 不在树上就添加
        if ( epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, ch->fd(), &ev) == -1) {
            perror("epoll_ctl add failed!");
            exit(-1);
        }
        ch->set_in_epoll();
    }
    
}
void Epoll::del_channel(Channel* ch) {
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, ch->fd(), NULL);
    // std::cout << "delete resources of client " << ch->sock()->fd() << std::endl;
    // 释放内存
    delete(ch->sock());
    delete(ch); 
}
std::vector<epoll_event> Epoll::loop(int timeout) {
    std::vector<epoll_event> evs;
    memset(evs_, 0, sizeof(evs_));
    int event_num = epoll_wait(epoll_fd_, evs_, MAXEVENT, timeout);
    if (event_num < 0) {
        perror("epoll_wait error!");
        exit(-1);
    } else if (event_num == 0) {
        return evs;
    }
    for (int i = 0; i < event_num; ++i) {
        evs.push_back(evs_[i]);
    }
    return evs;
}

std::vector<Channel*> Epoll::loop_ch(int timeout) {
    std::vector<Channel*> channels;
    memset(evs_, 0, sizeof(evs_));
    int event_num = epoll_wait(epoll_fd_, evs_, MAXEVENT, timeout);
    if (event_num < 0) {
        perror("epoll_wait error!");
        exit(-1);
    } else if (event_num == 0) {
        // std::cout << "epoll_wait timeout!" << std::endl;
        return channels;
    }
    for (int i = 0; i < event_num; ++i) {
        Channel* ch = (Channel*)evs_[i].data.ptr; // channel是ptr指向的内存
        ch->set_revents(evs_[i].events);
        channels.push_back(ch);
    }
    return channels;
}