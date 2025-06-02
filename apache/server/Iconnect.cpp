/*
* function that handling sockets sematics, epoll and connections
* in general
*/
#include "../includes/Iconnect.hpp"
#include <iostream>
#include <cstring>

void set_nonblocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

// create epoll object, adds socket fd to it
// returns epoll fd.
int createEpoll(struct epoll_event* event, int socketfd)
{
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        std::cerr << "Error creating epoll instance" << std::endl;
        return -1;
    }
    event->events = EPOLLIN;
    event->data.fd = socketfd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socketfd, event) == -1)
    {
        std::cerr << "Error adding server socket to epoll" << std::endl;
        return -1;
    }
    set_nonblocking(socketfd); // ! new code 02/6
    return epoll_fd;
}

int makePassiveSocket(struct sockaddr_in *server_addr)
{
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    std::memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = INADDR_ANY;
    server_addr->sin_port = htons(SERVER_PORT);

    if (bind(server_fd, (struct sockaddr *)server_addr, sizeof(*server_addr)) == -1)
    {
        std::cerr << "Error binding socket" << std::endl;
        return -1;
    }

    if (listen(server_fd, SOMAXCONN) == -1)
    {
        std::cerr << "Error listening on socket" << std::endl;
        return -1;
    }

    std::cout << "Server listening on port " << SERVER_PORT << std::endl;

    set_nonblocking(server_fd);
    return server_fd;
}

int makePassiveSocket(struct sockaddr_in *server_addr, int h, int p)
{
    int server_fd;
    // double loop test each host on ports...
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    std::memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = INADDR_ANY;
    server_addr->sin_port = htons(SERVER_PORT);

    if (bind(server_fd, (struct sockaddr *)server_addr, sizeof(*server_addr)) == -1)
    {
        std::cerr << "Error binding socket" << std::endl;
        return -1;
    }

    if (listen(server_fd, SOMAXCONN) == -1)
    {
        std::cerr << "Error listening on socket" << std::endl;
        return -1;
    }

    std::cout << "Server listening on port " << SERVER_PORT << std::endl;

    set_nonblocking(server_fd);
    return server_fd;
}

void manage_timeout(std::map<int , struct client> &activity){
    while (1)
    { for (auto it = activity.begin(); it != activity.end();) {
                if (it->second.start)
        if (time(NULL) - it->second.timestamp > 5) {
            std::cout << it->second.timestamp << std::endl;
            std::cout << "closing connection from timout_thread" << std::endl; 
            close(it->first);
            it = activity.erase(it);
        } else {
            ++it;
        }
    }
}
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

}
