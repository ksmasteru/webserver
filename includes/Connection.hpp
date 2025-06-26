#pragma once


#include "Response.hpp"
#include "Request.hpp"
#include "server.hpp"
 
// std::map <int, Connection*> map;
// each client has a connection.

// a connection should not be held more than 2min : to handle later/

// SO new update:
class Connection{
    private:
        //AResponse *resp;
        int _client_fd;
        // time connection started;
        struct timeval startTime;
        struct timeval connectionTime;
    public:
        bool _timeOut;
        bool _writeMode;
        Request request;
        struct timeval getTime()
        {
            return this->startTime;
        }
        struct timeval getConnectionTime()
        {
            return (this->connectionTime);
        }
        void    resetTime()
        {
            gettimeofday(&startTime, nullptr);
        }
        Response response;
        Connection(int client_fd, struct timeval& timeout){
            _timeOut = false;
            _client_fd = client_fd;
            startTime = timeout;
            _writeMode = false;
            gettimeofday(&connectionTime, nullptr);
        }
        void resetConnection(){
        }
        ~Connection(){}
};