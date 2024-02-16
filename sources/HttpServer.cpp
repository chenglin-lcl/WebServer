#include "HttpServer.h"


HttpServer::HttpServer(const std::string& ip, const uint16_t port) {

    // 关于内存释放，client的就是在断开连接的时候释放
    server_sock_ = new Socket(create_fd()); // 内存分配
    server_sock_->set_reuseaddr(true);
    server_sock_->set_tcpnodelay(true);
    InetAddr server_addr(ip, port);
    server_sock_->bind(server_addr);
    server_sock_->listen(1024);

    thread_pool_ = new ThreadPool(128, 1024, 2048);

    server_channel_ = new Channel(loop_.ep(), server_sock_, thread_pool_); // 内存分配
    // server_channel_->use_et();
    server_channel_->enable_read();
    server_channel_->set_callback(std::bind(&Channel::create_connect, server_channel_, server_sock_));
    
}
void HttpServer::start() {
    loop_.run();
}


HttpServer::~HttpServer() {
    std::cout << "delete server resources" << std::endl;
    // 释放内存
    thread_pool_->destoryPool();
    delete server_sock_;
    delete server_channel_;
    delete thread_pool_;
}