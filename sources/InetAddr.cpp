#include "InetAddr.h"


InetAddr::InetAddr() {

}
InetAddr::InetAddr(const std::string& ip, uint16_t port){   
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    addr_.sin_port = htons(port);
}


InetAddr::InetAddr(const sockaddr_in addr){
    addr_.sin_addr = addr.sin_addr;
    addr_.sin_port = addr.sin_port;

}
InetAddr::~InetAddr(){

}
const char* InetAddr::ip() const{
    return inet_ntoa(addr_.sin_addr);
}
uint16_t InetAddr::port() const{
    return ntohs(addr_.sin_port);
}   
const sockaddr* InetAddr::addr() const{
    return (sockaddr*)&addr_;
}
InetAddr& InetAddr::operator=(const InetAddr& other){
    if (this != &other) {
        addr_ = other.addr_;
    }
    return *this;
}