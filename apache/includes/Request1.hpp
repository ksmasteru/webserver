#pragma once

#include "webserver.hpp"
#define BUF_SIZE 1000

enum {
     start,
     ReadingRequestHeader,
     ReadingRequestBody,
     Done
}Request_state;
class Request1{
    private:
        char *buffer;
        int state;
    
    public:
        Request1()
        {
            state = start;
        }
        ~Request1();
        int getState(){return state;}
};
