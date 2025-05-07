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

int Server::run()
{
    // close connection after 5 second if no activity detected.
    std::thread timeout_thread(manage_timeout, std::ref(activity));
    timeout_thread.detach();
    /*int cfd = accept(data.sfd, NULL, NULL);
    if (cfd == -1)
    {
        printf ("couldnt connect\n");        exit (1);
    }*/
    struct client cl = {0, time(NULL)};
    //handleRequest(cfd);
    int client_fd;
    while (true)
    {
        int num_events = epoll_wait(data.epollfd, data.events, MAX_EVENTS, -1);
        if (num_events == -1)
        {
            std::cerr << "Error in epoll_wait" < std::endl;
            return EXIT_FAILURE;
        }

        for (int i = 0; i < num_events; i++)
        {
            if (data.events[i].data.fd == data.sfd)
            {
                client_fd = accept(data.sfd, (struct sockaddr *)&data.client_addr, &data.client_len);
                if (client_fd == -1)
                    continue;
                struct client cl = {0, time(NULL)};
                activity[client_fd] = cl;
                set_nonblocking(client_fd);
                data.event.events = EPOLLIN | EPOLLET;
                data.event.data.fd = client_fd;
                if (epoll_ctl(data.epollfd, EPOLL_CTL_ADD, client_fd, &data.event) == -1)
                    close(data.clientfd);
            }
            else
            {
                handleRequest(data.events[i].data.fd);
            }
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

void Server::parseRequest(const std::string& request, std::map<int , std::string>& map)
{
    int start = 5;
    int index = 0;
    size_t end ;
    std::string str;
    std::string reqcopy = request;
    while ((end = reqcopy.find('\n')) != std::string::npos)
    {
        if (!index)
        {
            str = reqcopy.substr(4, end - 14);
            map[index] = str;
            index++;
        }
        else
        {
            start = reqcopy.find(' ');
            map[index++] = reqcopy.substr(start + 1, (end - start) - 2);
        }
        reqcopy = reqcopy.substr(end + 1);
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

AResponse* Server::generateResponse(Request* req)
{
    if (req->getType() == "GET")
        return (new GetResponse("GET", req, &statusCodes));
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
    AResponse* resp = generateResponse(req);
    try {
            resp->makeResponse();
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
    if (send(efd , resp->getRes(), strlen(resp->getRes()), 0) == -1)
        std::cout << "send error" << std::endl;
    // reset timeout.
    /*if (req->isAlive())
    {
        std::cout << "reset client timeout" << std::endl; 
        activity[efd] = {1, NULL};
    }
    else*/
        close(efd);
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