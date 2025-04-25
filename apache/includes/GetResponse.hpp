#pragma once

#include "AResponse.hpp"

class GetResponse : public AResponse
{
    
    public:
        GetResponse(const std::string& type, Request *req, std::map<std::string, std::string>* status);
        virtual ~GetResponse();
        std::string generateHeader();
        std::string generateBody(const char* path);
        void  makeResponse();
        void  makeResponsa();
        std::string makeRspHeader();
        std::string RspStatusline(unsigned int code); 
        static std::string getTime();
        std::string pageBodyError(unsigned int code);
        std::string requestPageBody(const char* path);
        const char* getRes() const;
        bool isAlive() const;
};