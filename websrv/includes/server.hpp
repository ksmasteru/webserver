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
        ~Server();
        void loadstatuscodes(const char* filepath);
        int establishServer();
        int run();
        void handleRequest();
        Request& generateRequest();
        char *getRequest(int client_fd);
        void sendResponse(const AResponse* res);
        std::map<int, std::string> parseRequest(const std::string& request);
        AResponse* generateResponse(const Request&);
};

// handle request : getRequest --> getResponse --> sendRespond