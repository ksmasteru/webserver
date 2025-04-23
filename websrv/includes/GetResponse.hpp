#pragma once

#include "AResponse.hpp"

class GetResponse : public AResponse
{
    public:
        GetResponse(std::string type, Request& req);
        ~GetResponse(){}
        std::string generateHeader();
        std::string generateBody(const char* path);
        void  makeResponse();
        std::string makeRspHeader();
        static std::string getTime();
        std::string pageBodyError(unsigned int code);
        std::string requestPageBody(const char* path);
        const char* getRes() const;
};