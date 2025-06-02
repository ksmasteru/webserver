#pragma once
#include <string>
#define MAX_EVENTS 10
#define BUFFER_SIZE 1024
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <map>
#include <thread>   
#include "utils.hpp"
#include <ctime>
#include <sys/time.h>


typedef struct InetData{
    struct sockaddr_in server_fd, client_addr;
    socklen_t client_len;
    struct epoll_event event, events[MAX_EVENTS];
    int epollfd, clientfd;
    int sfd;
}t_InetData;

#include "ServerManager.hpp"
#include "Location.hpp"
#include "ConfigParser.hpp"
#include "Iconnect.hpp"
#define CLIENT_TIMEOUT 3