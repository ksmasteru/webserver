#pragma once
#include "Request.hpp"
#include "webserver.hpp"

class  AResponse{
    protected:
        const Request& _request;
        std::string _type;
        struct resp_h res_data;
        const char* resp_msg;
    public:
        AResponse(std::string type, Request& req) :_type(type), _request(req){
        }
        virtual ~AResponse() = 0;
        virtual std::string generateHeader() = 0;
        virtual std::string generateBody() = 0;
        virtual void makeResponse() = 0;
        virtual const char* getRes() const = 0;
};