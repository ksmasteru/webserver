#pragma once
#include "Request.hpp"
#include "webserver.hpp"

struct progress{
    int sentBytes;
    int sentTotal;
    int progress; // header sent   
};

class  AResponse{
    protected:
        Request* _request;
        std::map <std::string, std::string>* statuscodes;
        std::string _type;
        struct resp_h res_data;
        const char* resp_msg;// should point to
        std::ostringstream response;
        int _client_fd;
        struct progress _progress;
        // allocated string.
    public:
        AResponse(std::string type, Request* req, std::map<std::string, std::string>* status, int client_fd) :_type(type), _request(req), resp_msg(NULL), statuscodes(status)
        , _client_fd(client_fd){}
        AResponse(){}
        virtual ~AResponse(){
            if (this->resp_msg)
                delete []resp_msg;
            delete _request;
        }
        virtual void makeResponse(int cfd) = 0;
        virtual const char* getRes() const = 0;
        virtual bool isAlive () const = 0;
        virtual size_t  getSize() = 0;
};