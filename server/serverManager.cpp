#include "../includes/webserver.hpp"
#include "../includes/ServerManager.hpp"
#include "../includes/server.hpp"
#include "../includes/Iconnect.hpp"

ServerManager::ServerManager(std::vector<Server> &Servers) : servers(Servers)
{
    this->epoll_fd = epoll_create1(0);
    if (this->epoll_fd == -1)
        throw ("epoll create error");
}

int ServerManager::isServerSocket(int fd)
{
    for (size_t i = 0; i < servers.size(); i++)
    {
        for (size_t j = 0; j < servers[i].serverSockets.size(); j++)
        {
            if (servers[i].serverSockets[j] == fd)
                return i;
        }
    }
    return (-1);
}

int ServerManager::findServerIndex(std::string host, std::string port, std::vector<Server> servers)
{
    int targetPort = stringToInt(port);
    for (size_t i = 0; i < servers.size(); i++)
    {
        std::vector<std::string> hosts = servers[i].getHosts();
        std::vector<int> ports = servers[i].getPorts();    
        bool hostMatch = false;
        for (size_t j = 0; j < hosts.size(); j++)
        {
            if (hosts[j] == host)
            {
                hostMatch = true;
                break;
            }
        }    
        bool portMatch = false;
        for (size_t k = 0; k < ports.size(); k++)
        {
            if (ports[k] == targetPort)
            {
                std::cout << "port target : " << ports[k] << std::endl;
                portMatch = true;
                break;
            }
        }   
        if (hostMatch && portMatch)
            return static_cast<int>(i);
    }
    return -1;
}

void ServerManager::addToEpoll(int sfd, int mode)
{
    struct epoll_event ev;
    ev.events = mode;
    ev.data.fd = sfd;
    if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, sfd, &ev) == -1)
        throw("Epoll_Ctl failed\n");
    set_nonblocking(sfd);
}

void ServerManager::establishServers()
{
    // creating passive sockets for each server.
    for (size_t i = 0; i < servers.size(); i++)
    {
        std::vector<std::string> hosts = servers[i].getHosts();
        std::vector<int> ports = servers[i].getPorts();
        servers[i].setEpollfd(this->epoll_fd);
        for (size_t h = 0; h < hosts.size(); h++)
        {
            for (size_t p = 0; p < ports.size(); p++)
            {
                struct sockaddr_in serverAddr;
                int serverSocket = makePassiveSocket(&serverAddr, hosts[h], ports[p]);
                if (serverSocket == -1)
                {
                    continue;
                }
                servers[i].serverSockets.push_back(serverSocket); // added easy to manage | each server knows his sockets
                serverSockets.push_back(serverSocket);
                // add the new socket to epoll watch list
                addToEpoll(serverSocket, EPOLLIN);
                // add epollfd to servers
                std::cout << "Listening on " << hosts[h] << ":" << ports[p] << std::endl;
            }
        }
    }
}

// iterate each server's connected clients to find who matches
// the parameter fd : return server id on success ; -1 on error
int ServerManager::getTargetServer(int client_fd)
{
    for (size_t i = 0; i < servers.size(); i++)     
    {
        if (servers[i].clientExist(client_fd))
            return (i);
    }
    return (-1);
}

void ServerManager::closeAllSockets()
{
    for (size_t i = 0; i < servers.size(); i++)
        close (serverSockets[i]);
}

void ServerManager::closeTimedOutClients()
{
    for (size_t i = 0 ; i < servers.size(); i++)
        servers[i].unBindTimedOutClients();
}

void ServerManager::run()
{
    std::cout << "Server manager is running..." << std::endl;
    int serverIndex = -1;
    int targetServer = -1;
    
    while (true)
    {
        closeTimedOutClients();
        int num_events = epoll_wait(this->epoll_fd, this->epollEventsBuffer, MAX_EVENTS, 100);
        if (num_events == -1)
        {
            std::cout << "errno " << errno << std::endl;
            throw ("epoll_wait error\n");
        }
        for (int i = 0; i < num_events; i++)
        {
            if ((serverIndex = isServerSocket(epollEventsBuffer[i].data.fd)) != -1)
            {
                std::cout << "ServerIndex : " << serverIndex << "\n";
                this->servers[serverIndex].addNewClient(this->epoll_fd, epollEventsBuffer[i].data.fd);
                continue;
            }
            if ((targetServer = getTargetServer(epollEventsBuffer[i].data.fd)) == -1)
            {
                std::cout << "couldnt find a matching client in all servers : ";
                std::cout << epollEventsBuffer[i].data.fd << std::endl;
                continue ;
            }
            else if (epollEventsBuffer[i].events & EPOLLIN)
            {
                this->servers[targetServer].handleReadEvent(epollEventsBuffer[i].data.fd);
                continue;
            }
            else if(epollEventsBuffer[i].events & EPOLLOUT)
            {
                this->servers[targetServer].handleWriteEvent(epollEventsBuffer[i].data.fd);
            }
            else if (epollEventsBuffer[i].events & EPOLLHUP ||
                epollEventsBuffer[i].events & EPOLLERR)
            {
                this->servers[targetServer].handelSocketError(epollEventsBuffer[i].data.fd);
            }
        }
        // close all sockets
    }
    // also you should CLOSE ALL CLIENTS and open files.
}

void ServerManager::modifyEpollEvent(int fd, int mode)
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