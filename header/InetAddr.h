#ifndef _INETADDR_H
#define _INETADDR_H
#include <arpa/inet.h>
#include <iostream>
#include <string>

// 网络地址类，用于封装与结构体sockaddr_in相关的代码
class InetAddr {
public:
    InetAddr(const std::string& ip, uint16_t port); // 用于监听fd
    InetAddr();
    InetAddr(const sockaddr_in addr); // 用于通信fd
    ~InetAddr(); // 析构函数
    const char* ip() const; // 返回ip地址
    uint16_t port() const; // 返回端口号
    const sockaddr* addr() const; // 结构体转换
    InetAddr& operator=(const InetAddr& other);

private:
    sockaddr_in addr_;

};
#endif