#pragma once

#include "AResponse.hpp"

class GetResponse : public AResponse
{
    public:
        GetResponse(const std::string& type, Request *req, std::map<std::string, std::string>* status, int client_fd);
        virtual ~GetResponse();
        void  makeResponse(int cfd);
        std::string getTime();
        std::string makeRspHeader();
        std::string RspHeader(unsigned int cLength, unsigned int code);
        std::string RspStatusline(unsigned int code); 
        std::string requestPageBody(const char* path);
        void    sendPage(const char *path, int cfd, bool redirection);
        void    handleErrorPage(const char *path, int cfd);
        const char* getRes() const;
        size_t  getSize();
        bool isAlive() const;
};