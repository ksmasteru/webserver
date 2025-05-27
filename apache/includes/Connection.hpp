#pragma once

#include "Response.hpp"
#include "Request.hpp"
#include "server.hpp"

// std::map <int, Connection*> map;
// each client has a connection.

class Connection{
    private:
        //AResponse *resp;
        int _client_fd;
        // time connection started;
        struct timeval _timeout;
    public:
        Request request;
        Response response;
        Connection(int client_fd, struct timeval& timeout){
            _client_fd = client_fd;
            _timeout = timeout;
        }
        void resetConnection(){
        }
        ~Connection(){}
};