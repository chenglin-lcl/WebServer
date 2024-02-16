#include "Channel.h"


struct ArgList {
    Socket* sock_;
    Channel* channel_;
};



Channel::Channel() {
}

Channel::Channel(Epoll* p_epoll_tree, Socket* sock, ThreadPool* thread_pool):
    p_epoll_tree_(p_epoll_tree), 
    sock_(sock),
    thread_pool_(thread_pool) {
}

Channel::~Channel() {
    // 不要关闭fd,不要释放内存空间
    // Channel仅仅是使用这两个值
}

int Channel::fd() const {
    return sock_->fd();
}
Socket* Channel::sock() {
    return sock_;
}

void Channel::use_et() {
    events_ = events_|EPOLLET;
}
void Channel::enable_read() {
    events_ = events_|EPOLLIN;
    p_epoll_tree_->modify_channel(this);
}
void Channel::set_in_epoll() {
    in_epoll_ = true;
}
void Channel::set_revents(uint32_t ev) {
    revents_ = ev;

}
bool Channel::in_epoll() {
    return in_epoll_;

}
uint32_t Channel::events() {

    return events_;
}
uint32_t Channel::revents() {

    return revents_;
}



void connect_client_task(void* arg) {
    ArgList* info = (ArgList*)arg;
    // 构造函数可以放在成员函数里面
    // channel必须用动态内存分配
    Channel* client_channel = new Channel(info->channel_->p_epoll_tree_, info->sock_, info->channel_->thread_pool_); // 内存分配
    client_channel->use_et();
    client_channel->enable_read();
    //client_channel->set_callback(std::bind(&Channel::recv_message, client_channel));
    client_channel->set_callback(std::bind(&Channel::recv_http_request, client_channel));
    // std::cout << client_channel << std::endl;

}

void recv_task(void* arg) {
    ArgList* info = (ArgList*)arg;
        /*通信套接字接受数据*/
    char buff[4096] = {0};
    char tmp[1024] = {0};
    int ptr = 0; // 上一次的长度 
    while (true) { 
        ssize_t r_num = read(info->channel_->sock_->fd(), tmp, sizeof tmp);
        if (r_num > 0) {
            if (ptr + r_num < (int)sizeof(buff)) {
                memcpy(buff+ptr, tmp, r_num); // 将接受的数据存到buff中
            }
            ptr += r_num;
        } else if (r_num == -1 && errno == EINTR) {
            continue;
        } else if (r_num == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            // 读完了，开始解析数据
            HttpPRT http_prt(info->channel_->sock_->fd());
            http_prt.get_http_request_head(buff); // 提取出请求行
            http_prt.http_reponse(buff);
            break; 
        } else if (r_num == 0) { 
            // 对方已经关闭了
            std::cout << "client_fd = " << info->channel_->sock_->fd() << " closed!" << std::endl;
            info->channel_->p_epoll_tree_->del_channel(info->channel_);
            break;
        }
    }

}

void Channel::create_connect(Socket* server_sock) {
#if 0
    // 监听套接字建立连接
    InetAddr client_addr;
    // 服务器
    Socket* client_sock = new Socket(server_sock->accept(client_addr)); // 内存分配
    std::cout << "client fd: " << client_sock->fd() << std::endl;
    std::cout << "client ip: " << client_addr.ip() << std::endl;
    std::cout << "client port: " << client_addr.port() << std::endl;
    // 构造函数可以放在成员函数里面
    // channel必须用动态内存分配
    Channel* client_channel = new Channel(this->p_epoll_tree_, client_sock, thread_pool_); // 内存分配
    //client_channel->set_callback(std::bind(&Channel::recv_message, client_channel));
    client_channel->use_et();
    client_channel->enable_read();
    client_channel->set_callback(std::bind(&Channel::recv_http_request, client_channel));
    // std::cout << client_channel << std::endl;
    

#else
    // 监听套接字建立连接
    InetAddr client_addr;
    // 服务器
    Socket* client_sock = new Socket(sock_->accept(client_addr)); // 内存分配
    std::cout << "client fd: " << client_sock->fd() << std::endl;
    std::cout << "client ip: " << client_addr.ip() << std::endl;
    std::cout << "client port: " << client_addr.port() << std::endl;
    ArgList* arg_list = (ArgList*)malloc(sizeof(ArgList));
    arg_list->sock_ = client_sock;
    arg_list->channel_ = this;
    thread_pool_->addTask(connect_client_task, arg_list);
    
#endif
}


void Channel::recv_http_request() {
#if 0
    /*通信套接字接受数据*/
    char buff[4096] = {0};
    char tmp[1024] = {0};
    int ptr = 0; // 上一次的长度 
    while (true) { 
        ssize_t r_num = read(sock_->fd(), tmp, sizeof tmp);
        if (r_num > 0) {
            if (ptr + r_num < (int)sizeof(buff)) {
                memcpy(buff+ptr, tmp, r_num); // 将接受的数据存到buff中
            }
            ptr += r_num;
        } else if (r_num == -1 && errno == EINTR) {
            continue;
        } else if (r_num == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            // 读完了，开始解析数据
            HttpPRT http_prt(sock_->fd());
            http_prt.get_http_request_head(buff); // 提取出请求行
            http_prt.http_reponse(buff);
            break; 
        } else if (r_num == 0) { 
            // 对方已经关闭了
            std::cout << "client_fd = " << sock_->fd() << " closed!" << std::endl;
            p_epoll_tree_->del_channel(this);
            break;
        }
    }
#else
    ArgList* arg_list = (ArgList*)malloc(sizeof(ArgList));
    arg_list->channel_ = this;
    thread_pool_->addTask(recv_task, arg_list);

#endif 

}

void Channel::event_handle() {
    
    if (revents_ & EPOLLRDHUP) {
            // 检查对方是否关闭
            std::cout << "client_fd = " << sock_->fd() << " closed!" << std::endl;
            p_epoll_tree_->del_channel(this);
            
        } else if (revents_ & (EPOLLIN | EPOLLPRI)) {
            callback_();  
        } else if (revents_ & EPOLLOUT) { 
            // 有事件可以写
        } else {
            // 发生错误的情况
            std::cout << "client_fd = " << sock_->fd() << " error!" << std::endl;
            p_epoll_tree_->del_channel(this);
        }
}

void Channel::set_callback(std::function<void()> func) {
    callback_ = func;
}

