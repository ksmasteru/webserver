#include "../includes/webserver.hpp"
#include "../includes/server.hpp"
#include "../includes/Iconnect.hpp"
#define STATUS_PATH "/home/hes-saqu/Desktop/webserver/apache/server/codes.txt"
#include "../includes/AResponse.hpp"
#include "../includes/Response.hpp"
#include <fstream>

Server::Server()
{
    this->hasMaxBodySize = false;
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
    // here socket timeout is set. : 
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
    gettimeofday(&startTime, 0);
    Connection* new_client = new Connection(client_fd, startTime);
    // makes no sense.
    this->clients[client_fd] = new_client;
    // adding max body size attributes 8/8
    new_client->request.hasMaxBodySize = this->gethasMaxBodySize();
    new_client->request.maxBodySize = this->getMaxBodySize();
}

void Server::removeClient(int   fd)
{
    std::cout << "------------removing client of -----------" << fd << std::endl;
    close(fd);
    delete clients[fd];
    clients.erase(fd);
}

void Server::notAllowedPostResponse(int cfd)
{
    std::string allowedMethods = "Allow: GET, DELETE";
    std::ostringstream msg;
    /*HTTP/1.1 405 Method Not Allowed
    Content-Length: 0
    Date: Fri, 28 Jun 2024 14:30:31 GMT
    Server: ECLF (nyd/D179)
    Allow: GET, POST, HEAD*/
    msg << "HTTP/1.1 405 Method Not Allowed \r\n"
        << "Server: apache/2.4.41 (mac osx) \r\n"
        <<  "Content-Length: 0 \r\n" 
        << allowedMethods + " \r\n"
        << "\r\n";
    std::string resp = msg.str();
    if (send(cfd, resp.c_str(), resp.size(), MSG_NOSIGNAL) == -1)
    {
        std::cout << " failed to send " << cfd << std::endl;
        throw (cfd);
    }
}
void Server::handleReadEvent(int fd)
{
    std::cout << "new read event for fd " << fd << std::endl;
    if (clients.find(fd) == clients.end())
    {
        std::cout << "client of fd: " << fd << " was not found" << std::endl;
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
  /*  else if (readBytes == 0)
    {
        std::cout << "has read 0 bytes!!!"<< "\U0001F600" << std::endl;
        return ;
    }*/
    if (readBytes > 0)
    {
        std::cout << "resetting connection timer of " << fd << std::endl;
        this->clients[fd]->resetTime();
    }
    //std::cout << "Reead bytes are " << readBytes << std::endl;

    // new : this is a wrong approach ; sendinng is up to handleWriteEvent.
    switch (clients[fd]->request.getState())
    {
        case ReadingRequestHeader:
            try
            {
                clients[fd]->request.parseRequestHeader(buffer, readBytes, this->_locations);
            }
            catch (const char *msg)
            {
                std::cout << "parse request header failed" << std::endl;
                // here in case of error set response to done with a flag to type of error
                // to be send by response;
                
                // bellow is old code
                /*std::cout << msg << std::endl;
                sendBadRequest(fd);
                removeClient(fd);*/
                
                // new code
                std::cout << msg << std::endl;
                clients[fd]->request._requestErrors.badRequest = true;
                clients[fd]->request.MainState = Done;
            }
            catch (int n)
            {
                std::cout << "caught exeception code : " << n << std::endl;
                clients[fd]->request.MainState = Done;
                switch (n)
                {
                    case 405:
                        clients[fd]->request._requestErrors.notAllowed = true;
                        break;
                    case 413:
                        clients[fd]->request._requestErrors.ContentTooLarge = true;
                        break;
                    default:
                        break; 
                }
                //return (notAllowedPostResponse(fd));
                //removeClient(fd);
            }
            break;
        case ReadingRequestBody:
            try {
            clients[fd]->request.parseRequestBody(buffer, 0, readBytes, this->_locations);
            }
            catch (const char *msg)
            {
                std::cout << msg << std::endl;
                clients[fd]->request._requestErrors.badRequest = true;
                clients[fd]->request.MainState = Done;
                //sendBadRequest(fd);
                //removeClient(fd);
            }
            catch (int n)
            {
                std::cout << "caught exeception code : " << n << std::endl;
                clients[fd]->request.MainState = Done;
                switch (n)
                {
                    case 405:
                        clients[fd]->request._requestErrors.notAllowed = true;
                        break;
                    case 413:
                        clients[fd]->request._requestErrors.ContentTooLarge = true;
                        break;
                    default:
                        break; 
                }
            }
            break;
        default:
            std::cout << "waiting for the response to finish" << std::endl; // ??
            break;
    }
    // check if client still exist it might be deleted due to bad to request.
    
    if (clients[fd]->request.getState() == Done)/*!!!!!!!!!!new code*/
    {
        std::cout << "read event handled successfuly for target url " << clients[fd]->request.getRequestPath() << std::endl;
        // now open write mode acces for epoll.
        std::cout << "opened write permissions for fd: " << fd << std::endl;
        giveWritePermissions(fd);
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
    if (clients.find(fd) == clients.end())
    {
        std::cout << "client not found " << std::endl;
        return ;
    }
    else if (this->clients[fd]->_timeOut)
    {
        std::cout << "handle write event has detected a timeout on client : " << fd << std::endl;
        // new : for post if file didnt finish sending close it.
        if (clients[fd]->request.openRequestFile)
        {
            std::cout << "... closing post request file" << std::endl;
            // delete incomplete file.
            close(clients[fd]->request.RequestFile.fd);
            
            int a = unlink(clients[fd]->request.RequestFile.fileName.c_str());
            std::cout << "unlink status " << a << " for : " << clients[fd]->request.RequestFile.fileName << std::endl;
        }
        std::cout << "request state " << clients[fd]->request.getState() << std::endl;
        this->clients[fd]->response.sendTimedOutResponse(fd, &clients[fd]->request);
        // close connection;
        removeClient(fd);
        return ;
    }
    
    if (clients[fd]->request.getState() != Done) /*change into unique labels*/
    {
        //usleep(5000);
        return ;
    }
    clients[fd]->response.setRequest(&clients[fd]->request);
    // make reponse then write it.
    // too big of a file : state -> sending body :
    // attributees of response .
    // habdling bad requests!.
    if (clients[fd]->request._requestErrors.badRequest || 
        clients[fd]->request._requestErrors.ContentTooLarge || clients[fd]->request._requestErrors.notAllowed)
    {
        std::cout << "bad request flag detected" << std::endl;
        // exit here ? 405? handleBadRequest?
        // exit(1);
        try {
            clients[fd]->response.handleBadRequest(fd, &clients[fd]->request);}
        catch (int n)
        {
            std::cout << "handling connection failureo on " << n << std::endl;
            removeClient(fd);
        }
        return ;
    }
    
    else{
    try
    {
        if (clients[fd]->request.getType().compare("GET") == 0)
            clients[fd]->response.makeResponse(fd, &clients[fd]->request, _errorPages, _locations);
        else if (clients[fd]->request.getType().compare("POST") == 0 && clients[fd]->request.getState() == Done)
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
            std::cout << "resetting request and response of client : " << fd << std::endl;
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

/* it is a bad practice to give write permission only when you get a read event*/
void Server::giveWritePermissions(int fd)
{
    if (!this->clients[fd]->_writeMode)
    {
        std::cout << "Giving write permissions for socket of client " << fd << std::endl;
        struct epoll_event event;
        event.events = EPOLLOUT | EPOLLERR;
        event.data.fd = fd;
        if (epoll_ctl(this->getEpollfd(), EPOLL_CTL_MOD, fd, &event) == -1)
        {
            std::cout << "epoll ctl failed for epollfd " << this->getEpollfd() << std::endl;
            close(this->getEpollfd()); // Important: Close the epoll fd on error
            throw("epoll_ctl"); // this !.
        }
        this->clients[fd]->_writeMode = true;
    }
}

void Server::unBindTimedOutClients()
{
    // new 
    // iterate the clients map unbind those who timedout
    // NEW the timeout should depend on status;
    // sending header : 5sec;
    // sending BODY : 60sec;
    // TOTAL connection; 5min;
    std::map <int, Connection*>::iterator it;
    struct timeval curTime;
    gettimeofday(&curTime, 0);
    //std::cout << "unbind timeout clients launched..." << std::endl;
    for (it = clients.begin(); it != clients.end(); ++it)
    {
        // check total connection time; // will also work if sending.
        if ((curTime.tv_sec - it->second->getConnectionTime().tv_sec) >= CONNECTION_TIMEOUT)
        {
            //this->clients[it->first]->request.MainState = Done;
            this->clients[it->first]->_timeOut = true;
            giveWritePermissions(it->first);
            std::cout << "Timeout of total connection" << std::endl;
            // new code : we set a flag that determines if a timeout happened -> send timeout response.
            //removeClient(it->first);
            continue;
        }
        switch (this->clients[it->first]->request.getState())
        {
            case ReadingRequestHeader:
                if ((curTime.tv_sec - it->second->getTime().tv_sec) >= HEADER_TIMEOUT)
                {
                    giveWritePermissions(it->first);
                    //this->clients[it->first]->request.MainState = Done;
                    this->clients[it->first]->_timeOut = true;
                    std::cout << "client of fd: " << it->first << " has timedOut : HEADER" << std::endl;
                    //removeClient(it->first);
                }
                break;
            case ReadingRequestBody:
                std::cout << "timeout case request Body" << std::endl;
                if ((curTime.tv_sec - it->second->getTime().tv_sec) >= BODY_TIMEOUT)
                {
                    //this->clients[it->first]->request.MainState = Done;
                    this->clients[it->first]->_timeOut = true;
                    giveWritePermissions(it->first);
                    std::cout << "client of fd: " << it->first << " has timedOut : BODY" << std::endl;
                    // should send a timeout so remove client would be a flag ? when sending a response. ?
                    //removeClient(it->first);
                }
                break;
            default:
                break;
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
        throw std::runtime_error("Error: Invalid port number");
    _ports.push_back(port);
}

void Server::setEpollfd(int _epollfd){ this->epollfd = _epollfd;}
int  Server::getEpollfd(){return (this->epollfd);}
// void Server::setServerName(const std::string &name) { _serverNames.push_back(name); }
void Server::setClientMaxBodySize(size_t size) { _clientMaxBodySize = size; }
void Server::addErrorPage(int code, const std::string &path) { _errorPages[code] = path; }
void Server::addLocation(const Location &location) { _locations.push_back(location); }
std::vector<std::string> Server::getHosts() { return _hosts; }
std::vector<int> Server::getPorts() const { return _ports; }
// std::vector<std::string> &Server::getServerNames() { return _serverNames; }
size_t Server::getClientMaxBodySize() const { return _clientMaxBodySize; }
const std::map<int, std::string> &Server::getErrorPages() const { return _errorPages; }
const std::vector<Location> &Server::getLocations() const { return _locations; }
void Server::print() const
{
    std::cout << "Server:" << std::endl;
    for (size_t i = 0; i < _hosts.size(); i++)
        std::cout << " Host: " << _hosts[i] << std::endl;
    for (size_t i = 0; i < _ports.size(); i++)
        std::cout << "  Port: " << _ports[i] << std::endl;
    // for (size_t i = 0; i < _serverNames.size(); i++)
        // std::cout << "  ServerName : " << _serverNames[i] << std::endl;
    std::cout << "  Max Body Size: " << _clientMaxBodySize << std::endl;
    if (!_errorPages.empty())
    {
        std::cout << "  Error Pages:" << std::endl;
        /*std::map<int, std::string>::iterator it;
        for (it = _errorPages.begin(); it != _errorPages.end(); ++it)
            std::cout << "    " << it->first << " -> " << it->second << std::endl;*/
    }
    if (!_locations.empty())
    {
        std::cout << "  Locations:" << std::endl;
        for (size_t i = 0; i < _locations.size(); i++)
            _locations[i].print();
    }
}

void Server::removePort(std::string port)
{
    for (size_t i = 0; i < _ports.size(); i++)
    {
        if (_ports[i] == stringToInt(port))
            _ports.erase(_ports.begin() + i);
    }
}

void Server::removeHost(std::string host)
{
    for (size_t i = 0; i < _hosts.size(); i++)
    {
        if (_hosts[i] == host)
            _hosts.erase(_hosts.begin() + i);
    }
}

void Server::sethasMaxBodySize()
{
    this->hasMaxBodySize = true;
}

bool Server::gethasMaxBodySize()
{
    return this->hasMaxBodySize;
}


bool isValidConfigFile(int ac, char **av)
{
    if (ac != 2)
        return (false);
    std::string filename = av[1];
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

size_t Server::getMaxBodySize()
{
    return (this->_clientMaxBodySize);
}

int main(int ac, char **av)
{
    std::string confFile = (ac == 2) ? av[1] : "/home/ayoub/had/webserver/apache/webserv.conf";
    // if (!isValidConfigFile(ac, av))
        // return std::cerr << "Error: Config file must have a .conf extension" << std::endl, 1;
    // if (access(confFile.c_str(), R_OK) == -1)
    // {
    //     std::cout << "couldn't find config file" << std::endl;
    //     return (1);
    // }
    ConfigParser configParser;
    try {
        configParser.parse(confFile);
        configParser.printConfig();
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }
    // catch (const char *msg)
    // {
    //     std::cout << msg << std::endl;
    // }
    try 
    {
        ServerManager servManager(configParser.getServers());
        servManager.establishServers();
        try{
            servManager.run();
        }
        catch (const char *msg)
        {
            std::cerr << msg << std::endl;
            servManager.closeAllSockets();
            close(servManager.epoll_fd);
        }
    }
    catch(const char *msg)
    {
        std::cerr << msg << std::endl;
    }
}
