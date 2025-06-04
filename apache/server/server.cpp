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
    event.events = EPOLLIN | EPOLLHUP | EPOLLERR; // reading and error. 
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

void Server::addNewClient(int epoll_fd, int socket_fd)
{
    std::cout <<  "epollfd: " << epoll_fd;
    std::cout << " socket_fd "  << socket_fd << std::endl;
    int client_fd = accept(socket_fd, NULL, 0);
    // accept gives increasign values.
    if (client_fd == -1)
        throw ("client couldnt connect");
    std::cout << "new connecion on socket " << socket_fd;
    std::cout << " client id " << client_fd << std::endl;
    // clear
    struct timeval timeout;
    // set starting time in client, time map close the connection if times exces 10.
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
        throw std::runtime_error("setsockopt failed");
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLHUP | EPOLLERR; // reading and error. 
    event.data.fd = client_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1)
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
    {
        // close connection here.
        std::cout << "closed connection for read even of fd: " << fd << std::endl;
        removeClient(fd);
        return ;
    }
    else if (readBytes == 0)
    {
        //std::cout << "has read 0 bytes!!!"<< "\U0001F600" << std::endl;
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
        if (epoll_ctl(this->getEpollfd(), EPOLL_CTL_MOD, fd, &event) == -1)
        {
            close(this->getEpollfd()); // Important: Close the epoll fd on error
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
    try
    {
        if (clients[fd]->request.getType().compare("GET") == 0)
            clients[fd]->response.makeResponse(fd, &clients[fd]->request);
        else if (clients[fd]->request.getType().compare("POST") == 0)
            clients[fd]->response.successPostResponse(fd);
        else if (clients[fd]->request.getType().compare("DELETE") == 0)
            clients[fd]->response.deleteResponse(fd, &clients[fd]->request);
    }
    catch (int cfd) // if a send opeartion fails immmediately close the connection.
    {
        std::cout << "handling connection failure on " << cfd << std::endl;
        removeClient(cfd);
        return ;
    }
    catch (const char *msg) // this should be handled
    {
        std::cout << msg << std::endl;
        exit(1);
    }
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

void Server::handelSocketError(int fd)
{
    if (clients.find(fd) == clients.end())
    {
        std::cout << "client not found " << std::endl;
        return ;
    }
    std::cout << "------ERROR EVENT ON " << fd << std::endl;
    removeClient(fd);
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

// checks if parameter is a CONNECTED client in the server
bool Server::clientExist(int cfd)
{
    std::map<int, Connection*>::iterator it;
    for (it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->first == cfd)
            return (true);
    }
    return (false);
}

int Server::run()
{
    int client_fd;
    while (true)
    {
        unBindTimedOutClients();
        int num_events = epoll_wait(data.epollfd, data.events, MAX_EVENTS, -1);
        if (num_events == -1)
        {
                throw("off events!");
        }
       // std::cout << "num event ist " << num_events << std::endl;
        // delete timedout  clients;
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
            else if (data.events[i].events & EPOLLHUP || data.events[i].events & EPOLLERR)
            {
                handelSocketError(data.events[i].data.fd);
            }
            // handle epollERr and epollhup
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

void Server::setHost(const std::string &host)
{
    _hosts.push_back(host);
}

void Server::setPort(int port)
{
    if (port > 65535 || port < 1)
        throw std::runtime_error("Error: Invalid port number: " + std::to_string(port));
    _ports.push_back(port);
}

void Server::setEpollfd(int _epollfd){ this->epollfd = _epollfd;}
int  Server::getEpollfd(){return (this->epollfd);}
void Server::setServerName(const std::string &name) { _serverNames.push_back(name); }
void Server::setClientMaxBodySize(size_t size) { _clientMaxBodySize = size; }
void Server::addErrorPage(int code, const std::string &path) { _errorPages[code] = path; }
void Server::addLocation(const Location &location) { _locations.push_back(location); }
std::vector<std::string> Server::getHosts() { return _hosts; }
std::vector<int> Server::getPorts() const { return _ports; }
std::vector<std::string> &Server::getServerNames() { return _serverNames; }
size_t Server::getClientMaxBodySize() const { return _clientMaxBodySize; }
const std::map<int, std::string> &Server::getErrorPages() const { return _errorPages; }
const std::vector<Location> &Server::getLocations() const { return _locations; }
void Server::print() const
{
    std::cout << "Server:" << std::endl;
    for (auto &host : _hosts)
        std::cout << "  Host: " << host << std::endl;
    for (int i = 0; i < _ports.size(); i++)
        std::cout << "  Port: " << _ports[i] << std::endl;
    for (const auto &name : _serverNames)
        std::cout << "  Server Name: " << name << std::endl;
        std::cout << "  Max Body Size: " << _clientMaxBodySize << std::endl;
    if (!_errorPages.empty())
    {
        std::cout << "  Error Pages:" << std::endl;
        for (const auto &page : _errorPages)
        {
            std::cout << "    " << page.first << " -> " << page.second << std::endl;
        }
    }
    if (!_locations.empty())
    {
        std::cout << "  Locations:" << std::endl;
        for (const auto &loc : _locations)
        {
            loc.print();
        }
    }
}

void Server::removePort(std::string port)
{
    for (int i = 0; i < _ports.size(); i++)
    {
        if (_ports[i] == stringToInt(port))
            _ports.erase(_ports.begin() + i);
    }
}

void Server::removeHost(std::string host)
{
    for (int i = 0; i < _hosts.size(); i++)
    {
        if (_hosts[i] == host)
            _hosts.erase(_hosts.begin() + i);
    }
}

bool isValidConfigFile(int ac, char **av)
{
    if (ac != 2)
        return (false);
    std::string filename = av[1];
    // blabla.conf
    std::cout << "filename ist " << filename << std::endl;
    size_t dot = filename.rfind('.');
    if (dot == std::string::npos)
        return (false);
    std::string extension = filename.substr(dot);
    if (extension !=  ".conf")
        return (false);
    if (access(av[1], R_OK) == -1)
    {
        std::cout << "couldn't find config file" << std::endl;
        return (false);
    }
    return (true);
}

int main(int ac, char **av)
{
    if (ac != 2)
        return 1;
    std::string confFile = av[1];
    if (!isValidConfigFile(ac, av))
        return std::cerr << "Error: Config file must have a .conf extension" << std::endl, 1;
    if (access(av[1], R_OK) == -1)
    {
        std::cout << "couldn't find config file" << std::endl;
        return (1);
    }
    ConfigParser configParser;
    try {
        configParser.parse(confFile);
        configParser.printConfig();
        ServerManager servManager(configParser.getServers());
        servManager.establishServers();
        servManager.run();
    }
    catch (const char *msg)
    {
        std::cout << msg << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}