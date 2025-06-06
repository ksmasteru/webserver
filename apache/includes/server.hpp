/*
*   A simple class for webserver
*
*/
#pragma once

#include "webserver.hpp"
#include "Request.hpp"
#include "AResponse.hpp"
#include "Connection.hpp"

class Connection;
class Server{
    private:
        std::map<std::string, std::string> statusCodes;
        Server(Server& rhs);
        Server& operator=(Server& rhs);
        bool loadedStatusCodes;
        std::map<int, struct client> activity;
        t_InetData data;
        std::map <int, Connection*> clients;
    public:
        Server();
        ~Server(){
        }
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
};

// handle request : getRequest --> getResponse --> sendRespond