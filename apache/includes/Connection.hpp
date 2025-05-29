#pragma once

#include "Response.hpp"
#include "Request.hpp"
// #include "server.hpp"

// std::map <int, Connection*> map;
// each client has a connection.
class Server;
class Connection{
    private:
        // 
        // add the associated server to generate a proper response depending on the server's configuration.
        // and i you asking why not each port is in a different server object?
        // because they have the same configuration, so we can use the same server object.
        Server* _server;
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