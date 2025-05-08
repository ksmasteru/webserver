#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <map>
#include <thread>   
#include "utils.hpp"
#include <ctime>

#define MAX_EVENTS 10
#define BUFFER_SIZE 1024
#include <string>
typedef std::map<std::string, std::string> dstring_map;
typedef struct InetData{
    struct sockaddr_in server_fd, client_addr;
    socklen_t client_len;
    struct epoll_event event, events[MAX_EVENTS];
    std::map<int, struct client> activity;
    int epollfd, clientfd;
    int sfd;
}t_InetData;

struct resp_h{
    unsigned int clength; // content length
    unsigned int totallength;
    const char* keepAlive;
    unsigned int status;
    std::string contentType;
};

#endif 