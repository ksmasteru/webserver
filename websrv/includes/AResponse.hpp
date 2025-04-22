#pragma once
#include "Request.hpp"
#include "webserver.hpp"

class  AResponse{
    protected:
        const Request& _request;
        std::string _type;
        struct resp_h res;
    public:
        AResponse(std::string type, Request& req) :_type(type), _request(req){
            this->res = fillRes_h();
        }
        virtual ~AResponse() = 0;
        struct resp_h fillRes_h();
        std::string generateHeader();
        std::string generateBody();
        std::string generateResponse();
        std::string getTime();
};