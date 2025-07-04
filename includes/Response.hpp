#pragma once

#include "AResponse.hpp"
#include <ctime>
#include "Iconnect.hpp"
#include "Location.hpp"
#include <string>

#define BUFFER_SIZE 1024

enum  ResponseState{
    sendingheader,
    sendingBody,
    ResponseDone
};

struct resp_settings{
    bool GET;
    bool POST;
    bool DELETE;
    int Locationindex;
    bool redirected; // e.g : from /images --> /images/
};

class Response : public AResponse
{
    private:
        int sentBytes;
        int fileOffset;
        ResponseState state;
        bool    chunked;
        bool    openfile;
        int fd;
        bool settings_set;
        bool path_set;
        std::string filePath;
        struct resp_settings settings;

    public:
        Response(const std::string& type, Request *req, std::map<std::string, std::string>* status, int client_fd);
        Response();
        virtual ~Response();
        void makeResponse(int cfd, Request* req, std::map<int, std::string> &errorPages, std::vector<Location> &locations);
        void    getFileReady(int fd);
        std::string getTime();
        std::string makeRspHeader();
        std::string RspHeader(long long cLength, unsigned int code, Request*);
        std::string RspStatusline(unsigned int code);
        std::string requestPageBody(const char* path);
        std::string getPagePath(std::string,  std::vector<Location>);
        void    sendHeader(const char *, int, bool, Request*);
        void    sendPage(const char *path, int cfd, bool redirection, Request*);
        void    handleErrorPage(const char *path, int cfd, Request*);
        const char* getRes() const;
        size_t  getSize();
        void setResponseSettings(Location& _location, int);
        int getFd(const char *);
        bool isAlive() const;
        void sendChunkHeader (int, int);
        void successPostResponse(int, Request);
        int getState(){return this->state;}
        void setState(ResponseState st){
            state = st;
        }
        void reset()
        {
            std::cout << "reseting Response..." << std::endl;
            this->sentBytes = 0;
            this->fileOffset = 0;
            this->state = sendingheader;
            this->openfile = false;
            this->settings_set = false;
            this->path_set = false;
            this->settings.redirected = false;
            this->chunked = false;
        }
        void deleteResponse(int, Request*);
        void sendNotFoundPage(const char* path, int cfd, bool redir, Request*);
        std::string getPagePath(const char *, std::vector<Location>);
        void notAllowedGetResponse(int);
        void accessDeniedResponsePage(std::map<int, std::string>&);
        void notFoundResponsePage(std::map<int, std::string>&);
        bool handlePathRedirection(std::string, std::vector<Location>&);
        std::string getPagePath2(std::string , std::vector<Location>&);
        std::string getPath();
        void errorResponsePage(int, std::map<int, std::string>&, int , Request*);
        std::string getFolderName(const std::string& path);
        void redirectResponse(int, const char*);
        void handleBadRequest(int, Request*);
        void sendTimedOutResponse(int, Request& );
        void sendCgiResponse(int, Request*);

        // new code for cgi response
        void setCgiBody(const std::string& body);
        void addHeader(const std::string& key, const std::string& value);
        void setStatusCode(int code);
        void setContentType(const std::string& contentType);
        const std::string& getCgiBody() const;
        const std::map<std::string, std::string>& getCgiHeaders() const;
        int getStatusCode() const;
        bool isCgiResponse() const;
        void setBody(const std::string& body);
        std::string buildCgiResponse();
        void resetCgiData();
        
        bool hasCgiHeader(const std::string& headerName) const;
        void handleCgiRequest(const std::string&, int, Request*, std::map<int, std::string>&);
        std::string getCgiHeader(const std::string& headerName) const;
        void mergeCgiResponse();
        bool isCgiScript(const std::string &requestPath);
        std::string getStatusMessage(int);
        

        // cookies
        void addCookiesHeader(std::ostringstream &ofs, Request request);
        // slak ajmi ma3andk kidir
        void addCookiesHeaderp(std::ostringstream &ofs, Request* request);
};