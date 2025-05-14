#pragma once
#include <string>
#include <map>
#pragma once
#include <iostream>

enum mainState{
    ReadingRequestHeader,
    ReadingRequestBody,
    Done
};

enum subState{
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
        name,
        OWS1,
        val,
        CR,
        LF,
        doneParsing
};

class  Request{ // read event.
    std::string RawRequest;
    std::string type;
    std::string qkey;
    std::string qvalue;
    mainState MainState;
    subState SubState;
    char        httpGreater;
    char        httpMinor;
    int totalReadBytes;
    int _bytesread;
    bool keep_alive;
    std::string targetUri;
    std::map<std::string, std::string> queries;
    std::map<std::string, std::string> headers;
    Request(Request& rhs, int bytesread);
    Request& operator=(Request& rhs);
    std::map<int, std::string> parse_map;
    public:
    Request();
    std::string getHttpVersion();
    void addtoheaders(std::string& key, std::string& val);
    std::string getRequestPath();
    const std::string& getRawRequest() const;
    const std::string& getType() const;
    void parseRequestHeader(char* request, int readBytes);
    void parseRequestLine(char *request, int readBytes, int &offset);
    void parseRequestBody(char *request, int ReadBytes);
    std::string getMapAtIndex(unsigned int index);
    void printRequestLine();
    void printHeaderFields();
    void setConnectionType();
    bool isAlive();
    void reset(){
        std::cout << "reset request ..." << std::endl;
        RawRequest.clear();
        type.clear();
        qkey.clear();
        qvalue.clear();
        MainState = ReadingRequestHeader;
        SubState = start;
        totalReadBytes = 0;
        _bytesread = 0;
    }
    int getState(){
        return this->MainState;
    }
    ~Request(){}
};
