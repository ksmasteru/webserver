#pragma once
#include "Request.hpp"
#include "webserver.hpp"

class  AResponse{
    protected:
        Request* _request;
        std::string _type;
        struct resp_h res_data;
        const char* resp_msg;// should point to
        // allocated string.
    public:
        AResponse(std::string type, Request* req) :_type(type), _request(req), resp_msg(NULL){
        }
        virtual ~AResponse(){
            delete []resp_msg;
            delete _request;
        }
        virtual std::string generateHeader() = 0;
        virtual std::string generateBody(const char* path) = 0;
        virtual void makeResponse() = 0;
        virtual const char* getRes() const = 0;
        virtual bool isAlive () const = 0;
};