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

// pass in the actual host:port combo.
// why is the first host empty ?
int makePassiveSocket(struct sockaddr_in *server_addr, std::string host, int port)
{
    int server_fd;
    // double loop test each host on ports...
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }
    std::cout << "passive socket made for port : " << port << " and host " << host << std::endl;
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    std::memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    //server_addr->sin_addr.s_addr = INADDR_ANY;
    server_addr->sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &(server_addr->sin_addr)) <= 0)
    {
        perror("Invalid address");
        close(server_fd);
        return -1;
    }
    std::cout << "transforming host : " << host << " into " << server_addr->sin_addr.s_addr << std::endl;
    std::cout << "transforming port : " << port << " into " <<  server_addr->sin_port << std::endl;
    if (bind(server_fd, (struct sockaddr *)server_addr, sizeof(*server_addr)) == -1)
    {
        std::cerr << "Error binding socket port: " << port << std::endl;
        std::cerr <<  "host " << host << std::endl;
        return -1; /*Should be a cancelation point ?*/
    }

    if (listen(server_fd, SOMAXCONN) == -1)
    {
        std::cerr << "Error listening on socket" << std::endl;
        return -1;
    }
    //std::cout << "Server listening on port " << SERVER_PORT << std::endl;
    set_nonblocking(server_fd);
    return server_fd;
}

