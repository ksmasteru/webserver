#pragma once

#include "AResponse.hpp"

struct File{
    std::vector<char> *buffer;
    int offset;
    int totalLength;
    int sentBytes;
    char *extension;
};

class GetResponse : public AResponse
{
    private:
        struct File _FILE;
    public:
        GetResponse(const std::string& type, Request *req, std::map<std::string, std::string>* status);
        virtual ~GetResponse();
        void  makeResponse();
        void  makeResponsa();
        std::string handleBinaryFile(const char* path, std::string& extension);
        std::string getTime();
        std::string makeRspHeader();
        void   setFileReady(const char* path);
        void   readFile();
        void    sendFile();
        std::string RspHeader(unsigned int cLength, unsigned int code);
        std::string RspStatusline(unsigned int code); 
        std::string requestPageBody(const char* path);
        const char* getRes() const;
        size_t  getSize();
        bool isAlive() const;
};