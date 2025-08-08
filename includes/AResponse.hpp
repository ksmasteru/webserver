// norms checked
#pragma once
#include "Request.hpp"

struct progress
{
    int sentBytes;
    int sentTotal;
    int progress;
};

struct resp_h{
    long long clength;
    long long totallength;
    const char* keepAlive;
    unsigned int status;
    std::string contentType;
    std::string extension;
};


class AResponse
{
    protected:
    struct resp_h res_data;
    std::ostringstream response;
    std::string cgi_body;
    int status_code;
    std::string _type;
    Request *_request; // get rid of this ? this causes a lot of issues
    const char *resp_msg; // should point to
    std::map<std::string, std::string> *statuscodes;
    int _client_fd;
    bool is_cgi_response;
    std::map<std::string, std::string> cgi_headers;
    struct progress _progress;
    public:
    AResponse(std::string type, Request* req, std::map<std::string, std::string>* status, int client_fd);
    AResponse();
    struct resp_h getResData();
    virtual ~AResponse();
    virtual const char* getRes() const = 0;
    virtual bool isAlive () const = 0;
    virtual size_t  getSize() = 0;
    void setCgiBody(const std::string& body);
    std::string getStatusMessage(int code);
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
    std::string getCgiHeader(const std::string& headerName) const;
    void mergeCgiResponse();
    void setRequest(Request *);
};