#include "HttpPRT.h" 

static int hex_to_dec(char ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    return 0;
}

static void decode_msg(char* to, char* from) {
    for (; *from != '\0'; ++to, ++from) {
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) {
            *to = hex_to_dec(from[1]) * 16 + hex_to_dec(from[2]);
            from += 2;
        } else {
            *to = *from;
        }

    }
    *to = '\0'; // 截断
}
static const char* get_file_type(const char* name) {
    // 从右往左找，遇到点结束
    const char* dot = strrchr(name, '.');
    if (dot == NULL) {
        return "text/plain; charset=utf-8";
    }
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0) {
        return "text/html; charset=utf-8";
    }
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0) {
        return "image/jpeg";
    }
    if (strcmp(dot, ".gif") == 0 ) {
        return "image/gif";
    }
    if (strcmp(dot, ".png") == 0 ) {
        return "image/png";
    }
    if (strcmp(dot, ".css") == 0 ) {
        return "text/css";
    }
    if (strcmp(dot, ".au") == 0 ) {
        return "audio/basic";
    }
    if (strcmp(dot, ".wav") == 0 ) {
        return "audio/wav";
    }
    if (strcmp(dot, ".avi") == 0 ) {
        return "video/x-msvideo";
    }
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0) {
        return "video/quicktime";
    }
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0) {
        return "video/quicktime";
    }
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0) {
        return "model/vrml";
    }
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0) {
        return "audio/midi";
    }
    if (strcmp(dot, ".mp3") == 0 ) {
        return "audio/mpeg";
    }
    if (strcmp(dot, ".ogg") == 0 ) {
        return "application/ogg";
    }
    if (strcmp(dot, ".pac") == 0 ) {
        return "application/x-ns-proxy-autoconfig";
    }
    if (strcmp(dot, ".mp4") == 0 ) {
        return "video/mp4";
    }


     return "text/plain; charset=utf-8";

}



HttpPRT::HttpPRT(int fd): fd_(fd) {


}

HttpPRT::~HttpPRT() {


}

void HttpPRT::get_http_request_head(char* data) {
    // 对于请求报文，只需要头部就行了
    const char* ptr = strstr(data, "\r\n");
    int len = ptr - data;
    data[len] = '\0';
    
}

int HttpPRT::http_reponse(const char* lines) {
    // get /xxx/1.txt http/1.1
    // 解析请求行
    // std::cout << lines <<  std::endl;
    char method[12] = {0};
    char path[1024] = {0};
    sscanf(lines, "%[^ ] %[^ ]", method, path);
    if (strcasecmp(method, "get") != 0) {
        // 如果不是get方法，那么就不用解析
        return -1;
    }
    decode_msg(path, path);
    // std::cout << path <<  std::endl;
    // 根据path获得对应于服务器中的文件或者文件夹
    char* file = NULL;
    if (strcmp(path, "/") == 0) {
        // get / http/1.1
        file = (char*)"./";
    } else {
        // get /xxx/1.txt http/1.1
        file = path + 1;
        if (file[strlen(file)-1] == '/') {
            file[strlen(file)-1] = '\0';
        }
        // 这是为了去掉最后面的斜线
    }
    std::cout << "请求: " << file <<  std::endl;

    // 判断file是文件还是文件夹
    struct stat st;
    int ret = stat(file, &st);
    // std::cout << "ret: " << ret <<  std::endl;
    if (ret == -1) {
        // 文件不存在，响应404报文
        http_response_head(404, "Not Found", get_file_type(".html"), -1);
        http_response_file("404.html");
        return 0;
    }
    if (S_ISDIR(st.st_mode)) {
        // 是目录，将目录发送回去
        // std::cout << "目录" <<  std::endl;
        http_response_head(200, "ok", get_file_type(".html"), -1);
        http_send_dir(file);
    } else {
        // 发送文件
        std::cout << "文件大小: " << st.st_size << std::endl;
        http_response_head(200, "ok", get_file_type(file), st.st_size);
        http_response_file(file);
    }

    return 0;
}

int HttpPRT::http_send_dir(const char* dir_name) {
    /* 格式
    <html>
        <head>
            <tile>test</title>
        </head>

        <body>
            <table>
                <tr> // 表示行
                    <td></td> // 表示列
                </tr>
            </table>
        </body>

    </html>
    */
    char buff[4096] = {0};
    sprintf(buff, "<html><head><tile>%s</title></head><body><table>", dir_name);
    struct dirent** name_list;
   // 该函数就是在当前目录下搜索文件，返回文件个数，第三个为过滤规则，第四个是排序方式
    int num = scandir(dir_name, &name_list, NULL, alphasort);
    for (int i = 0; i < num; ++i) {
        char* name = name_list[i]->d_name;
        char file_path[1024] = {0};
        sprintf(file_path, "%s/%s", dir_name, name);
        struct stat st;
        stat(name, &st);
        if (S_ISDIR(st.st_mode)) {
            // sprintf(buff+strlen(buff), "<tr><td>%s</td><td>%ld</td></tr>", name, st.st_size);
            // 如果你只发送上述数据，那么这些数据你只能看，你没办法点击，操作他。想要解决这个问题
            // 使用<a href="">name</a>
            // 是目录，可以点击进目录
            sprintf(buff+strlen(buff), 
                "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>", 
                name, name, st.st_size);
        } else {
            // 不是目录，
            sprintf(buff+strlen(buff), 
                "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>", 
                name, name, st.st_size);
        }
        send(fd_, buff, strlen(buff), 0);
        memset(buff, 0, sizeof(buff));
        free(name_list[i]);       
    }
    // 发送结束标签
    sprintf(buff, "</table></body></html>");
    send(fd_, buff, strlen(buff), 0);
    free(name_list);

    return 0;
}


int HttpPRT::http_response_head(int status, const char* descr, const char* type, int length) {
    char buff[4096] = {0};
    // 状态行
    // 例如 http/1.1 200 ok
    sprintf(buff, "http/1.1 %d %s\r\n", status, descr);
    // 响应头
    sprintf(buff + strlen(buff), "content-type: %s\r\n", type);
    // 这里会多一个空行
    sprintf(buff + strlen(buff), "content-length: %d\r\n\r\n", length);
    // 发送头部数据
    send(fd_, buff, strlen(buff), 0);
    return 0;
}




int HttpPRT::http_response_file(const char* file_name) {
    const int BUFFER_SIZE = 1024;
    std::ifstream file(file_name, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_name << std::endl;
        return 0;
    }

    // std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    char buffer[BUFFER_SIZE];
    while (!file.eof()) {
        file.read(buffer, BUFFER_SIZE);
        ssize_t bytes_read = file.gcount();
        ssize_t bytes_sent = send(fd_, buffer, bytes_read, MSG_NOSIGNAL);
        usleep(500);
        if (bytes_sent < 0) {
            std::cerr << "Error sending file." << std::endl;
            break;
        }
    }
    file.close();
    return 0;
}




