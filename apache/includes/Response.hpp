#pragma once

#include "AResponse.hpp"

enum  ResponseState{
    sendingheader,
    sendingBody,
    ResponseDone
};

class AResponse;
class Response : public AResponse
{
    private:
        int sentBytes;
        int fileOffset;
        ResponseState state;
        bool    chunked;
        bool    openfile;
        int fd;
    public:

        Response(const std::string& type, Request *req, std::map<std::string, std::string>* status, int client_fd);
        Response();
        virtual ~Response();
        void  makeResponse(int cfd, Request*);
        void    getFileReady(int fd);
        std::string getTime();
        std::string makeRspHeader();
        std::string RspHeader(unsigned int cLength, unsigned int code);
        std::string RspStatusline(unsigned int code); 
        std::string requestPageBody(const char* path);
        void    sendHeader(const char *, int, bool);
        void    sendPage(const char *path, int cfd, bool redirection);
        void    handleErrorPage(const char *path, int cfd);
        const char* getRes() const;
        size_t  getSize();
        int getFd(const char *);
        bool isAlive() const;
        void sendChunkHeader (int, int);
        void successPostResponse(int);
        int getState(){return this->state;}
        void setState(ResponseState st){
            state = st;
        }
        void reset()
        {
            std::cout << "reseting Response..." << std::endl;
            sentBytes = 0;
            fileOffset = 0;
            state = sendingheader;
            openfile = false;
        }
        void deleteResponse(int, Request*);
        void sendNotFoundPage(const char* path, int cfd, bool redir);
        // 
        void handleCgiRequest(const std::string& scriptPath, int cfd, Request* req);
        void sendCgiResponse(int cfd);
        // Cgi cgiHandler(req, this, scriptPath, 5);
};