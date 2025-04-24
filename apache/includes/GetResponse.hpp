#pragma once

#include "AResponse.hpp"

class GetResponse : public AResponse
{
    
    public:
        GetResponse(const std::string& type, Request *req);
        virtual ~GetResponse();
        std::string generateHeader();
        std::string generateBody(const char* path);
        void  makeResponse();
        void  makeResponsa();
        std::string makeRspHeader();
        static std::string getTime();
        std::string pageBodyError(unsigned int code);
        std::string requestPageBody(const char* path);
        const char* getRes() const;
        bool isAlive() const;
};