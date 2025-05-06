#pragma once
#include <string>
#include <map>
#pragma once


class  Request{
    private:
        enum{
        start = 0,
        method_name,
        after_method_space,
        request_uri,
        question_mark,
        query_equal_mark,
        querykey,
        queryValue,
        spaceafterurl,
        HTTPword,
        httpgreat,
        dot,
        httpminor,
        request_uri_done,
    }state;
    std::string RawRequest;
    std::string type;
    std::string qkey;
    std::string qvalue;
    char        httpGreater;
    char        httpMinor;
    int _bytesread;
    bool keep_alive;
    std::string targetUri;
    int offset;
    std::map<std::string, std::string> queries;
    std::map<std::string, std::string> headers;
    Request(Request& rhs, int bytesread);
    Request& operator=(Request& rhs);
    std::map<int, std::string> parse_map;
    public:
    Request(char *buffer, int bytesread);
    std::string getHttpVersion();
    void addtoheaders(std::string& key, std::string& val);
    std::string getRequestPath();
    const std::string& getRawRequest() const;
    const std::string& getType() const;
    void parseRequest(char* request);
    bool parseRequestLine(char *request);
    std::string getMapAtIndex(unsigned int index);
    void printRequestLine();
    void printHeaderFields();
    void setConnectionType();
    bool isAlive(); 
    ~Request(){}
};

// reponse should take a request ptr ?