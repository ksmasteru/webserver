#pragma once
#include "Request.hpp"
#include "webserver.hpp"
#include <vector>

class  AResponse{
    protected:
        Request* _request;
        std::map <std::string, std::string>* statuscodes;
        std::string _type;
        struct resp_h res_data;
        std::vector<char> *resp_msg;
        std::ostringstream response;
        size_t filledBytes;
        size_t totalBytes;
        // allocated string.
    public:
        AResponse(std::string type, Request* req, std::map<std::string, std::string>* status) :_type(type), _request(req), resp_msg(nullptr), statuscodes(status)
        ,filledBytes(0){}
        virtual ~AResponse(){
            if (this->resp_msg)
                delete []resp_msg;
            delete _request;
        }
        virtual void makeResponse() = 0;
        virtual const char* getRes() const = 0;
        virtual bool isAlive () const = 0;
        virtual size_t  getSize() = 0;
};