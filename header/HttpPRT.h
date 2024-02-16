#ifndef _HTTPPRT_H
#define _HTTPPRT_H
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <cassert>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <dirent.h>
#include <iostream>
#include <fstream>


class HttpPRT {
public:
    HttpPRT(int fd);
    ~HttpPRT();
    void get_http_request_head(char* lines);
    int http_reponse(const char* lines);
    int http_response_head(int status, const char* descr, const char* type, int length);
    int http_response_file(const char* file_name);
    int http_send_dir(const char* dir_name);

private:
    int fd_;

};
#endif