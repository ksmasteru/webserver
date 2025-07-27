#pragma once

#include "webserver.hpp"
#include "Iconnect.hpp"

// handles epoll :
class ServerManager
{
private:
    ServerManager() = delete; // ?

public:
    std::vector<Server> servers;
    // replace serverSockets from vector<int> to vector<vectore<int>> where each element is a server and in each element the sockets of the server

    std::vector<int> serverSockets; // replace it by serversockets inside servers (view diagram).
    std::map<int, Connection *> clients;
    int epoll_fd;
    struct epoll_event epollEventsBuffer[MAX_EVENTS];
    ServerManager(std::vector<Server> &Servers);
    //bool isServerSocket(int fd);
    int isServerSocket(int fd);
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
    void run();
    int getTargetServer(int);
    void closeAllSockets();
    void closeTimedOutClients();
};