#ifndef _CHANNEL_H
#define _CHANNEL_H
#include "Epoll.h"
#include <iostream>
#include "InetAddr.h"
#include "Socket.h"
#include <functional>
#include "HttpPRT.h"
#include "ThreadPool.h"

// event结构体中的data中的fd成员改成ptr的成员
// 相比直接使用fd来说，ptr可以指向class对象，携带的信息更多
void connect_client_task(void* arg);
void recv_task(void* arg);
class Epoll; // 必须声明
class Channel {
public:
    Channel(Epoll* p_epoll_tree, Socket* sock, ThreadPool* thread_pool);
    Channel(); 
    ~Channel();
    int fd() const;
    Socket* sock();
    void use_et();
    void enable_read();
    void set_in_epoll();
    void set_revents(uint32_t ev);
    bool in_epoll();
    uint32_t events();
    uint32_t revents(); // 已发生事件
    void event_handle(); // 检测监听文件描述符
    void create_connect(Socket* server_sock);
    void set_callback(std::function<void()> func);
    void recv_http_request(); // 接受http请求，并作出响应
    friend void connect_client_task(void* arg);
    friend void recv_task(void* arg);

private:
    Epoll* p_epoll_tree_ = nullptr; // Channel与红黑树是多对一的关系
    Socket* sock_ = nullptr; // 一个fd对应一个channel, 一对一的关系
    bool in_epoll_ = false; // Channel是否在红黑树上
    uint32_t events_ = 0; // 要添加的事件
    uint32_t revents_ = 0; // fd_已经发生的事件
    std::function<void()> callback_;
    ThreadPool* thread_pool_;
};



#endif