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
        int loadstatuscodes(const char* filepath);
        int establishServer();
        char *getRequest(int client_fd);
        std::map<int, std::string> parseRequest(const char *request);
        AResponse* generateResponse(Request&);
};