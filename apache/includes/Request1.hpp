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
        char httpGreater;
        char httpMinor;
        std::map<std::string, std::string>queries;
        std::map<std::string, std::string>headers;
        std::string type;
        std::string qkey;
        std::string qvalue;
    public:
        Request1()
        {
            state = start;
        }
        ~Request1();
        int getState(){return state;}
        std::string getType(){return this->type;}
        void parseRequest();
        void readRequest(int);
        bool parseMethodLine();
        void addtoHeaders();
        std::string gge
};
