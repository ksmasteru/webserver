#include "../includes/webserver.hpp"
#include "../includes/server.hpp"
#include "../includes/Iconnect.hpp"
#define STATUS_PATH "/home/hes-saqu/Desktop/webserver/apache/server/codes.txt"
#include "../includes/AResponse.hpp"
#include "../includes/Response.hpp"
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

/*testing : changing epoll watchlist.*/
void Server::addNewClient()
{
    int client_fd = accept(data.sfd, NULL, 0);
    // accept gives increasign values.
    if (client_fd == -1)
        throw ("client couldnt connect");
    std::cout << "new client Connected : id " << client_fd << std::endl;
    // clear
    struct timeval timeout;
    // set starting time in client, time map close the connection if times exces 10.
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    if (setsockopt(data.sfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
        throw std::runtime_error("setsockopt failed");
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLERR; // reading and error. 
    event.data.fd = client_fd;
    if (epoll_ctl(data.epollfd, EPOLL_CTL_ADD, client_fd, &event) == -1)
        throw ("epoll_ctl fail");
    set_nonblocking(client_fd);
    // create a connection object and att it to <fd, connection>map;
    struct timeval startTime;
    gettimeofday(&startTime, nullptr);
    Connection* new_client = new Connection(client_fd, startTime);
    // makes no sense.
    this->clients[client_fd] = new_client;
}

void Server::removeClient(int   fd)
{
    std::cout << "------------removing client of -----------" << fd << std::endl;
    close(fd);
    delete clients[fd];
    clients.erase(fd);
}

void Server::handleReadEvent(int fd)
{
    //std::cout << "new read event for fd " << fd << std::endl;
    if (clients.find(fd) == clients.end())
    {
        //std::cout << "client of fd: " << fd << " was not found" << std::endl;
        return ;
    }
    char buffer[BUFFER_SIZE];
    // receiv.
    int readBytes = recv(fd, buffer, BUFFER_SIZE, 0);
    if (readBytes == -1)
        throw ("recv failed");
    else if (readBytes == 0)
    {
        std::cout << "has read 0 bytes!!!"<< "\U0001F600" << std::endl;
        return ;
    }
    if (readBytes > 0)
    {
        std::cout << "resetting connection timer of " << fd << std::endl;
        this->clients[fd]->resetTime();
    }
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
                sendBadRequest(fd);
                removeClient(fd);
                return ;
            }
            break;
        case ReadingRequestBody:
            try {
            clients[fd]->request.parseRequestBody(buffer, 0,readBytes);
            }
            catch (const char *msg)
            {
                std::cout << msg << std::endl;
                sendBadRequest(fd);
                removeClient(fd);
                return ;
            }
            break;
        default:
            std::cout << "waiting for the response to finish" << std::endl;
            break;
    }
    if (clients[fd]->request.getState() == Done)/*!!!!!!!!!!new code*/
    {
        std::cout << "read event handled successfuly for target url " << clients[fd]->request.getRequestPath() << std::endl;
        // now open write mode acces for epoll.
        struct epoll_event event;
        event.events = EPOLLOUT | EPOLLERR;
        event.data.fd = fd;
        if (epoll_ctl(data.epollfd, EPOLL_CTL_MOD, fd, &event) == -1)
        {
            close(data.epollfd); // Important: Close the epoll fd on error
            throw("epoll_ctl");
        }
    }
}

void Server::sendBadRequest(int fd)
{
    std::ostringstream msg;

    msg << "HTTP/1.1 400 Bad Request \r\n"
        << "\r\n";
    if (send(fd, msg.str().c_str(), msg.str().length(), MSG_NOSIGNAL) == - 1)
        throw("send error");        
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
    else if (clients[fd]->request.getType().compare("DELETE") == 0)
        clients[fd]->response.deleteResponse(fd, &clients[fd]->request);
    // reset timeout timer
    clients[fd]->resetTime();
    if (clients[fd]->response.getState() == ResponseDone /*&& clients[fd]->request.isAlive()*/) //  the last reponse completed  the file
    {
        // afte writing we could remove it from epoll ?
        // we wont write anything until we get a request.
        if (!clients[fd]->request.isAlive()) /*keep alive*/
            removeClient(fd);
        else /*revoke write permission*/
        {
            // just reset everything.
            clients[fd]->request.reset();
            clients[fd]->response.reset();
            // block write?
        }
    }
}

void Server::unBindTimedOutClients()
{
    // iterate the clients map unbind those who timedout
    std::map <int, Connection*>::iterator it;
    struct timeval curTime;
    gettimeofday(&curTime, nullptr);
    for (it = clients.begin(); it != clients.end(); ++it)
    {
        if ((curTime.tv_sec - it->second->getTime().tv_sec) >= CLIENT_TIMEOUT)
        {
            std::cout << "client of fd: " << it->first << " has timedOut" << std::endl;
            removeClient(it->first);
        }
    }
}
/*Checks of a client of a given ID connection was closed
which will require add him as a new client with a new fd
or else send call will SIGPIPE*/
bool Server::clientWasRemoved(int toFind)
{
    //std::cout << "to find " << toFind << std::endl;
    std::map<int, Connection*>::iterator it;

    for (it = this->clients.begin() ;it != this->clients.end(); ++it)
    {
        if (toFind == it->first)
             return (false);
    }
    std::cout << "----------client was removed previously------------" << std::endl;
    return (true);
}

int Server::run()
{
    int client_fd;
    while (true)
    {
        int num_events = epoll_wait(data.epollfd, data.events, MAX_EVENTS, -1);
        if (num_events == -1)
        {
                throw("off events!");
        }
        // delete timedout  clients;
        unBindTimedOutClients();
        for (int i = 0; i < num_events; i++)
        {
            if ((data.events[i].data.fd == data.sfd))
            {
                std::cout << "server socket" << std::endl;
                addNewClient();
                continue;
            }
            else if (data.events[i].events & EPOLLIN)
            {
                /*if (clientWasRemoved(data.events[i].data.fd))
                {
                    addNewClient();
                    continue;
                }*/
                handleReadEvent(data.events[i].data.fd);
            }
            if (data.events[i].events & EPOLLOUT)
            {
                // if client connection was closed skip!
                handleWriteEvent(data.events[i].data.fd);
            }
        }
    }
    std::cout << "loop exited" << std::endl;
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
    try{
        apache.run();}
    catch (const char *error_msg)
    {
        std::cout << "error : " << error_msg << std::endl;
    }
}