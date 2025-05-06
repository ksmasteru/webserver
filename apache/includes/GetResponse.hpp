#pragma once

#include "AResponse.hpp"

class GetResponse : public AResponse
{
    public:
        GetResponse(const std::string& type, Request *req, std::map<std::string, std::string>* status);
        virtual ~GetResponse();
        void  makeResponse();
        void  makeResponsa();
        std::string getTime();
        std::string makeRspHeader();
        std::string RspHeader(unsigned int cLength, unsigned int code);
        std::string RspStatusline(unsigned int code); 
        std::string requestPageBody(const char* path);
        const char* getRes() const;
        bool isAlive() const;
};