#include "../includes/webserver.hpp"
#include "../includes/server.hpp"
#include "../includes/Iconnect.hpp"
#define STATUS_PATH "/home/hes-saqu/Desktop/webserver/apache/server/codes.txt"
#include "../includes/AResponse.hpp"
#include "../includes/GetResponse.hpp"
#include <fstream>

Server::Server()
{
    std::cout << "Launching Server Apache v1.0" << std::endl;
    //memset(&(this->data),0, sizeof(t_InetData)); //!!
}

// socket + f
int Server::establishServer()
{
    //struct sockaddr_in server_addr, client_addr;
    //socklen_t client_len = sizeof(client_addr);
    data.client_len = sizeof(data.client_addr);
    data.sfd = makePassiveSocket(&data.server_fd);
    if (data.sfd == -1)
        throw ("");
    data.epollfd = createEpoll(&data.event, data.sfd);
    if (data.epollfd == -1)
        throw ("epoll");
    /*try {
        loadstatuscodes(STATUS_PATH);
    }
    catch (const char* error)
    {
        throw (error);
        exit (1);
    }
    this->loadedStatusCodes = true;
    */return (0);
}

void Server::addNewClient()
{
    int client_fd = accept(data.sfd, NULL, 0);
    if (client_fd == -1)
        throw ("client couldnt connect");
    std::cout << "new client Connected " << std::endl;
    // clear
    struct timeval timeout;
    // set starting time in client, time map close the connection if times exces 10.
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    if (setsockopt(data.sfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
        throw std::runtime_error("setsockopt failed");
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLOUT | EPOLLERR; 
    event.data.fd = client_fd;
    if (epoll_ctl(data.epollfd, EPOLL_CTL_ADD, client_fd, &event) == -1)
        throw ("epoll_ctl fail");
    set_nonblocking(client_fd);
    // create a connection object and att it to <fd, connection>map;
    Connection* new_client = new Connection(client_fd, timeout);
    this->clients[client_fd] = new_client;
}

void Server::handleReadEvent(int fd)
{
    //std::cout << "new read event for fd " << fd << std::endl;
    if (clients.find(fd) == clients.end())
    {
        std::cout << "client of fd: " << fd << " was not found" << std::endl;
        return ;
    }
    char buffer[BUFFER_SIZE];
    // receiv.
    int readBytes = recv(fd, buffer, BUFFER_SIZE, 0);
    if (readBytes == -1)
        throw ("recv failed");
    //std::cout << "Reead bytes are " << readBytes << std::endl;
    switch (clients[fd]->request.getState())
    {
        case ReadingRequestHeader:
            try {
            clients[fd]->request.parseRequestHeader(buffer, readBytes);
            }
            catch (const char *msg)
            {
                std::cout << msg << std::endl;
                exit(1);
            }
            break;
        case ReadingRequestBody:
            try {
            clients[fd]->request.parseRequestBody(buffer, 0,readBytes);
            }
            catch (const char *msg)
            {
                std::cout << msg << std::endl;
                exit(1);
            }
            break;
        default:
            std::cout << "waiting for the response to finish" << std::endl;
            break;
    }
    if (clients[fd]->request.getState() == Done)
        std::cout << "read event handled successfuly for target url " << clients[fd]->request.getRequestPath() << std::endl;
}

void Server::handleWriteEvent(int fd)
{
    // writing to client of fd.
    // minium write operation should cover the header.
    //std::cout << "received a write event on client of fd " << fd << std::endl;
    if (clients.find(fd) == clients.end())
    {
        std::cout << "client not found " << std::endl;
        return ;
    }
    if (clients[fd]->request.getState() != Done) /*change into unique labels*/
    {
        //std::cout << "client of fd not ready to receive data " << std::endl;
        //usleep(5000);
        return ;
    }
    // make reponse then write it.
    // too big of a file : state -> sending body :
    // attributees of response .
    // handling get first.
    if (clients[fd]->request.getType().compare("GET") == 0)
        clients[fd]->response.makeResponse(fd, &clients[fd]->request);
    else if (clients[fd]->request.getType().compare("POST") == 0)
        clients[fd]->response.successPostResponse(fd);
    if (clients[fd]->response.getState() == ResponseDone /*&& clients[fd]->request.isAlive()*/) //  the last reponse completed  the file
    {
        //std::cout << "client is still alive ..." << std::endl;
        //clients[fd]->request.reset();
        //clients[fd]->response.reset();
        std::cout << "deleting connection ..." << std::endl;
        close(fd);
        delete clients[fd];
        clients.erase(fd);
    }
}
int Server::run()
{
    int client_fd;
    while (true)
    {
        int num_events = epoll_wait(data.epollfd, data.events, MAX_EVENTS, -1);
        if (num_events == -1)
            return EXIT_FAILURE;
        for (int i = 0; i < num_events; i++)
        {
            if (data.events[i].data.fd == data.sfd)
            {
                std::cout << "server socket" << std::endl;
                addNewClient();
                continue;
            }
            else if (data.events[i].events & EPOLLIN)
                handleReadEvent(data.events[i].data.fd);
            if (data.events[i].events & EPOLLOUT)
                handleWriteEvent(data.events[i].data.fd);
        }
    }
    close(data.sfd);
    close(data.epollfd);
    return 0;
}

void Server::loadstatuscodes(const char* filepath)
{
    std::ifstream ifs;
    ifs.open(filepath);
    if (!ifs)
    {
        std::cerr << "couln't load status codes file " << filepath<< std::endl; 
        return ;
    }
    std::string line;
    std::string key;
    std::string value;
    while (std::getline(ifs, line))
    {
        key = line.substr(0, 3);
        value = line.substr(5, sizeof(line) - 5);
        this->statusCodes.insert(std::pair<std::string, std::string>(key, value));
    }
}


int main()
{
    Server apache;
    try {
        apache.establishServer();
    }
    catch (const char *error_msg)
    {
        std::cout << "error : " << error_msg << std::endl;
    }
    apache.run();
}