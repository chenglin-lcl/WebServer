#ifndef _HTTPSERVER_H
#define _HTTPSERVER_H
#include "EventLoop.h"
#include <functional>
#include "Channel.h"
#include <memory>
#include "ThreadPool.h"

class HttpServer {
public:

    HttpServer(const std::string& ip, const uint16_t port);
    ~HttpServer();
    void start();

private:
    EventLoop loop_;
    Socket* server_sock_;
    Channel* server_channel_;
    ThreadPool* thread_pool_;
};
#endif