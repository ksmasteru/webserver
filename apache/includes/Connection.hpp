#pragma once

#include "AResponse.hpp"
#include "Request.hpp"
#include "server.hpp"

// std::map <int, Connection*> map;
// each client has a connection.
enum{
    start,
    readingRequest,
    doneReadingRequest,
    ResponseHeaderSent,
    sendingResponseBody,
    done,
}connection_state;
class Connection{
    private:
        AResponse *resp;
        Request request;
        int client_fd;
        // time connection started;
        struct timeval timeout;
        int state;
    public:
        Connection();
        ~Connection();
};