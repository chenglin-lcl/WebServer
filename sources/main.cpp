#include "HttpServer.h"
#include <unistd.h>
int main(int argc, char* argv[]) {

    /* 进程路径切换 */
    chdir("/home/lcl/server_home");
    const char *ip = "192.168.31.167";
    uint16_t port = 20000;
    
    HttpServer server(ip, port);
    server.start();
    return 0;

}