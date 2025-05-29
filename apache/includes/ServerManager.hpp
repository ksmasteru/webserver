#pragma once

class ServerManager
{
private:
    ServerManager() = delete;

public:
    std::vector<Server> servers;
    std::vector<int> serverSockets;
    std::map<int, Connection *> clients;
    int epoll_fd;

    ServerManager(std::vector<Server> &Servers)
    {
        servers = Servers;
        epoll_fd = epoll_create1(0);
        if (epoll_fd == -1)
        {
            perror("epoll_create1");
            exit(EXIT_FAILURE);
        }
    }
    void establishServers()
    {
        for (size_t i = 0; i < servers.size(); ++i)
        {
            std::vector<std::string> hosts = servers[i].getHosts();
            std::vector<int> ports = servers[i].getPorts();

            for (size_t h = 0; h < hosts.size(); ++h)
            {
                for (size_t p = 0; p < ports.size(); ++p)
                {
                    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
                    if (serverSocket < 0)
                    {
                        std::cerr << "Error creating socket" << std::endl;
                        continue;
                    }
                    int opt = 1;
                    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
                    struct sockaddr_in serverAddr;
                    serverAddr.sin_family = AF_INET;
                    serverAddr.sin_port = htons(ports[p]);

                    if (inet_pton(AF_INET, hosts[h].c_str(), &serverAddr.sin_addr) <= 0)
                    {
                        perror("Invalid address");
                        close(serverSocket);
                        continue;
                    }

                    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
                    {
                        std::cerr << "Error binding socket to " << hosts[h] << ":" << ports[p] << std::endl;
                        close(serverSocket);
                        continue;
                    }

                    if (listen(serverSocket, SOMAXCONN) < 0)
                    {
                        std::cerr << "Error listening on socket " << hosts[h] << ":" << ports[p] << std::endl;
                        close(serverSocket);
                        continue;
                    }
                    set_nonblocking(serverSocket);
                    serverSockets.push_back(serverSocket);
                    std::cout << "Listening on " << hosts[h] << ":" << ports[p] << std::endl;
                }
            }
        }
        for (size_t i = 0; i < serverSockets.size(); i++)
        {
            addToEpoll(serverSockets[i], EPOLLIN);
        }
    }
    void addToEpoll(int fd, int mode)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1)
        {
            perror("fcntl F_GETFL");
            close(fd);
            exit(EXIT_FAILURE);
        }

        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        {
            perror("fcntl F_SETFL");
            close(fd);
            exit(EXIT_FAILURE);
        }

        struct epoll_event ev;
        ev.events = mode;
        ev.data.fd = fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1)
        {
            perror("epoll_ctl: listen_sock");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }
    void modifyEpollEvent(int fd, int mode)
    {
        struct epoll_event ev;
        ev.events = mode;
        ev.data.fd = fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1)
        {
            perror("epoll_ctl: listen_sock");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }
    void run()
    {
        // run the eventloop here where you have serverSockets and epollfd modify and add to epoll member functions 
    }
};