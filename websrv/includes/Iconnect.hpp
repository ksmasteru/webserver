#pragma once

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>
#include <thread>   
#define SERVER_PORT 8080
struct client
{
    int start;
    time_t timestamp;
};
void set_nonblocking(int sockfd);
int makePassiveSocket(struct sockaddr_in *server_addr);
int createEpoll(struct epoll_event* event, int socketfd);
void manage_timeout(std::map<int , struct client> &activity);