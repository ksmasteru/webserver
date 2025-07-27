/*
*   A simple class for webserver
*
*/
#pragma once

#include "webserver.hpp"
#include "Connection.hpp"
#include "Location.hpp"
#include "Iconnect.hpp"

typedef struct InetData{
    struct sockaddr_in server_fd, client_addr;
    socklen_t client_len;
    struct epoll_event event, events[MAX_EVENTS];
    int epollfd, clientfd;
    int sfd;
}t_InetData;

class Connection;
class Server{
    private:
        std::vector<int> _ports;
        std::vector<std::string> _hosts;
        std::vector<std::string> _serverNames;
        size_t _clientMaxBodySize;
        std::map <int, std::string> _errorPages; // new
        std::vector<Location> _locations; // new 
        std::map<std::string, std::string> statusCodes;
        bool loadedStatusCodes;
        t_InetData data;
        std::map <int, Connection*> clients;
        int epollfd;
    public:
        Server();
        ~Server(){
        }
        
        std::vector<int> serverSockets;
        // new code to support multiple servers + config file.
        void setEpollfd(int);
        int     getEpollfd();
        void setHost(const std::string &);
        void setPort(int port);
        void setServerName(const std::string &name);
        void setClientMaxBodySize(size_t size);
        void addErrorPage(int code, const std::string &path);
        void addLocation(const Location &location);
        //setters
        std::vector<std::string> getHosts();
        std::vector<int> getPorts() const;
        std::vector<std::string> &getServerNames();
        size_t getClientMaxBodySize() const;
        const std::map<int, std::string> &getErrorPages() const;
        const std::vector<Location> &getLocations() const;
        void removePort(std::string port);
        void removeHost(std::string host);
        void print(); 
        void addNewClient(int, int);
        // end of new code.
    
        void handleReadEvent(int);
        void unBindTimedOutClients();
        void handleWriteEvent(int);
        void removeClient(int);
        void addNewClient();
        void loadstatuscodes(const char* filepath);
        int establishServer();
        int run();
        void handleRequest(int efd);
        void handelSocketError(int);
        char *getRequest(int client_fd);
        void parseRequest(const std::string& request, std::map<int, std::string>& map);
        void sendBadRequest(int);
        bool clientWasRemoved(int);
        bool clientExist(int);
        void notAllowedPostResponse(int cfd);
        void giveWritePermissions(int);
};
