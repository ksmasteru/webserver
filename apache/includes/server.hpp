/*
*   A simple class for webserver
*
*/
#pragma once

#include "webserver.hpp"
#include "Request.hpp"
#include "AResponse.hpp"

class Server{
    private:
        std::map<std::string, std::string> statusCodes;
        Server(Server& rhs);
        Server& operator=(Server& rhs);
        bool loadedStatusCodes;
        t_InetData data;
    public:
        Server();
        ~Server(){
        }
        void loadstatuscodes(const char* filepath);
        int establishServer();
        int run();
        void handleRequest(int efd);
        Request* generateRequest(int efd);
        char *getRequest(int client_fd);
        void sendResponse(AResponse* res);
        void parseRequest(const std::string& request, std::map<int, std::string>& map);
        AResponse* generateResponse(Request*);
};

// handle request : getRequest --> getResponse --> sendRespond