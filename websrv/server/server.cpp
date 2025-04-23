#include "../includes/webserver.hpp"
#include "../includes/server.hpp"
#include "../includes/Iconnect.hpp"

Server::Server()
{
    std::cout << "Launching Server Apache v1.0" << std::endl;
    memset(&(this->data),0, sizeof(t_InetData)); //!!
}

// socket + f
int Server::establishServer()
{
    //struct sockaddr_in server_addr, client_addr;
    //socklen_t client_len = sizeof(client_addr);
    data.client_len = data.client_addr;
    int server_fd = makePassiveSocket(&server_addr);
    if (server_fd == -1)
        throw ("");
    int client_fd, epoll_fd;
    //struct epoll_event event, events[MAX_EVENTS];
    int epoll_fd = createEpoll(&event, server_fd);
}

int Server::loadstatuscodes(const char* filepat)
{
   
}

char* Server::getRequest(int client_fd)
{

}

std::map<int, std::string> parseRequest(const char* request)
{

}

AResponse* generateResponse(Request& req)
{

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
}