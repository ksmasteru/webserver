#pragma once

#include "GetResponse.hpp"
#include "Request.hpp"
#include "server.hpp"

// std::map <int, Connection*> map;
// each client has a connection.
enum ConnectionState{
    start,
    readingRequestHeader,
    readingRequestBody,
    sendingResponse,
    hold,
    done,
};

class Connection{
    private:
        //AResponse *resp;
        int _client_fd;
        // time connection started;
        struct timeval _timeout;
        ConnectionState state;
    public:
        Request request;
        GetResponse response;
        Connection(int client_fd, struct timeval& timeout){
            _client_fd = client_fd;
            _timeout = timeout;
            state = start;
        }
        ConnectionState getState()
        {
            return (this->state);
        }
        void resetConnection(){
        }
        ~Connection();
};