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
    event.events = EPOLLIN | EPOLLHUP | EPOLLERR; 
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
    switch (clients[fd]->getState())
    {
        case start:
            clients[fd]->request.parseRequestHeader(buffer, readBytes);
            break;
        case readingRequestHeader:
            clients[fd]->request.parseRequestHeader(buffer, readBytes);
            break;
        case readingRequestBody:
            clients[fd]->request.parseRequestBody(buffer, readBytes);
            break;
        case sendingResponse:
            if (clients[fd]->response.getState() == done)
            {
                clients[fd]->setState(done);
                break;
            }
        case done:
            clients[fd]->request.reset();
            // create a new request or should i wait for the previous to finish
            // store 
    }
    // if the user sends a new reques and he previous request wasnt answered but this request
    // on hold until the queued request is ansewred.
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
    // make reponse then writet it.
    // too big of a file : state -> sending body :
    // attributees of response .
    // handling get first.
    switch (this->clients[fd]->reponse.getSatate())
}

int Server::run()
{
    int client_fd;
    while (true)
    {
        int num_events = epoll_wait(data.epollfd, data.events, MAX_EVENTS, -1);
        std::cout << "num events is " << num_events << std::endl;
        if (num_events == -1)
            return EXIT_FAILURE;
        for (int i = 0; i < num_events; i++)
        {
            if (data.events[i].data.fd == data.sfd)
                addNewClient();
            else if (data.events[i].data.fd & EPOLLIN) /* data can be read */
                handleReadEvent(data.events[i].data.fd);
            else if (data.events[i].data.fd & EPOLLOUT) /* data can be sent*/
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


Request* Server::generateRequest(int efd)
{
    //struct client cl = data.activity[data.clientfd]; // :?
    char buffer[BUFFER_SIZE];
    //std::cout << "data client fd : " << data.clientfd << std::endl;
    ssize_t bytes_read = read(efd, buffer, BUFFER_SIZE - 1);
    //std::cout << "bytes read : " << bytes_read <<  "buffer : " << buffer << std::endl;
    if (bytes_read <= 0)
    {
        close(data.clientfd);
        throw ("read :: generateRequest");
    }
    buffer[bytes_read] = '\0';
    //std::ofstream ofs("get.txt");
    //ofs << buffer;
    //std::map<int, std::string> map;
    //std::string buf = buffer;
    //parseRequest(buf, map);
    //Request req(map, buf);
    // which class should allocate map and buffer?
    Request *req1 = new Request(buffer, bytes_read);
    
    return req1; // mablansh t returni this/
    /*
    if (requestBuffer.find("GET") != std::string::npos)
        return ();
    else if(Rawrequest.find("POST") != std::string::npos){
        std::cout << "request " << request;
        close(client_fd);
        return ;
    }*/
}

AResponse* Server::generateResponse(Request* req, int client_fd)
{
    if (req->getType() == "GET")
        return (new GetResponse("GET", req, &statusCodes, client_fd));
    /*else if (req.getType() == "POST")
        return (new PostResponse("POST", req));
    else if (req.getType() == "DELETE")
        return (new DelResponse("DELETE", req));*/
    //else
      //  return (new BadRequest(req)); // TODO
    std::cout << "bad request" << std::endl;
    return (nullptr);
}

void Server::sendResponse(AResponse* resp)
{
    std::cout << "send reponse called" << std::endl;
    // TODO chubked response isnt handled yet;
    if (send(this->data.clientfd, resp->getRes(), strlen(resp->getRes()), 0) == -1)
        std::cout << "send error" << std::endl;
}

void Server::handleRequest(int efd)
{
    // create thread here.
    // need epoll. 
    Request* req;
    try {
        req = generateRequest(efd);
    }
    catch (const char* error)
    {
        std::cout << error << std::endl;
        return ;
    }
    AResponse* resp = generateResponse(req, efd);
    try {
            resp->makeResponse(efd);
        }
    catch (const char* error)
    {
        std::cout << error << std::endl;
        exit (1); 
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    // format of response issue.
    //std::cout << "response\n" << resp->getRes() << std::endl;
    //exit(1);
    /*
    if (send(efd , resp->getRes(), resp->getSize(), 0) == -1)
        std::cout << "send error" << std::endl;
    else
        std::cout << "sent " << resp->getSize() << std::endl;
    if (req->isAlive())
    {
        std::cout << "reset client timeout" << std::endl; 
        activity[efd] = {1, NULL};
    }
    else
        close(efd);*/
    delete resp;
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