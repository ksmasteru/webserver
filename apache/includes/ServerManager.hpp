#pragma once

#include "webserver.hpp"
#include "server.hpp"
#include "Iconnect.hpp"

class ServerManager
{
private:
    ServerManager() = delete; // ?

public:
    std::vector<Server> servers;
    std::vector<int> serverSockets;
    std::map<int, Connection *> clients;
    int epoll_fd;

    ServerManager(std::vector<Server> &Servers);
    bool isServerSocket(int fd);
    int findServerIndex(std::string host, std::string port, std::vector<Server> servers);
    void establishServers();
    void addToEpoll(int fd, int mode);
    void modifyEpollEvent(int fd, int mode)
    {
        struct epoll_event ev;
        ev.events = mode;
        ev.data.fd = fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1)
        {
            perror("epoll_ctl: listen_sock");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }
};