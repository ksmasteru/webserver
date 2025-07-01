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

/*
bool ServerManager::isServerSocket(int fd)
{
    size_t lent = this->serverSockets.size();
    for (int i = 0; i < lent; i++)
    {
        if (fd == serverSockets[i])
            return (true);
    }
    return (false);
}*/

// return id of the server whose socket match the paramter
// returns -1 on failure
// each server has multiple sockets.
int ServerManager::isServerSocket(int fd)
{
    for (int i = 0; i < servers.size(); i++)
    {
        for (int j = 0; j < servers[i].serverSockets.size(); j++)
        {
            if (servers[i].serverSockets[j] == fd)
                return i;
        }
    }
    // for (int i = 0; i < serverSockets.size(); i++)
    // {
    //      if (serverSockets[i] == fd)
    //         return (i);
    // }
    return (-1);
}

// determine which server will have to handle the host port request combo.
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

// adds socket "sfd" to epoll watch list.
void ServerManager::addToEpoll(int sfd, int mode)
{
    struct epoll_event ev;
    ev.events = mode;
    ev.data.fd = sfd;
    if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, sfd, &ev) == -1)
        throw("Epoll_Ctl failed\n");/*shouldnt be a cancelaton point*/
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
                    throw ("Error making passive socket...\n"); /*this shouldnt be a cancelation point?*/
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

// epoll loop : handles connection on all servers sockets.
// determine which server is targeted and what type of connection action it should take.
// Each server handles its own clients tru  map<int, Connection*>
void ServerManager::run()
{
    std::cout << "Server manager is running..." << std::endl;
    int serverIndex = -1;
    int targetServer = -1;
    
    while (true)
    {
        // updated : sets flag on clients that timed out --> response send timeout and detachs;
        closeTimedOutClients();
        //std::cout << "out loop" << std::endl;
        int num_events = epoll_wait(this->epoll_fd, this->epollEventsBuffer, MAX_EVENTS, 100); // here it is blocking.
        // so fllow of exectuion stucks here : i guess it is a bad idea;
        //std::cout << "number of events is: " << num_events << std::endl;
        if (num_events == -1)
        {
            std::cout << "errno " << errno << std::endl;
            throw ("epoll_wait error\n");
        }   
        // one process handles all connection  NO DUPLICATE fds.
        for (int i = 0; i < num_events; i++)
        {
            //std::cout << "In loop" << std::endl;
            // check if connection happens on server socket.
            if ((serverIndex = isServerSocket(epollEventsBuffer[i].data.fd)) != -1)
            {
                std::cout << "ServerIndex : " << serverIndex << "\n";
                // here we have a problem the connection is in server index 0 but the output shows 2 to address the issue i will edit isServerSocket.
                // try catch this ?
                this->servers[serverIndex].addNewClient(this->epoll_fd, epollEventsBuffer[i].data.fd);
                continue;
            }
            // determine which server has to handle this
            if ((targetServer = getTargetServer(epollEventsBuffer[i].data.fd)) == -1) /*how can this fail ?*/
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