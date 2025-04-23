#pragma once

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define SERVER_PORT 8080
void set_nonblocking(int sockfd);
int makePassiveSocket(struct sockaddr_in *server_addr);
int createEpoll(struct epoll_event* event, int socketfd);
