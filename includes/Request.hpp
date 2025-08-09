#pragma once
#include <string>
#include <map>
#include <iostream>
#include <fcntl.h>
#include "../includes/utils.hpp"
#include "Location.hpp"

class Location;
enum Chunk_State{
    chunk_size,
    CR1,
    LF1,
    chunk_data,
    CR2,
    CR3,
    LF3,
    chunk_done
};

enum mainState{
    ReadingRequestHeader,
    ReadingRequestBody,
    Done
};
//api?user=alice&id=42
enum subState{
        start = 0,
        method_name,
        after_method_space,
        request_uri,
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
        parsingBody,
        doneParsing
};

enum Transfer_Type{
    NONE,
    Content_Length,
    Chunked
};

typedef struct s_FILE{
    std::string fileName;
    std::string uploadPath;
    int fd;
    unsigned long offset;
    Transfer_Type type;
    unsigned long size;
    Chunk_State state;
    std::string chunk_lent;
    size_t toWrite;// number of characters to write on a chuck
}t_FILE;

struct requestErrors{
    bool notAllowed;
    bool badRequest;
    bool ContentTooLarge;
    bool internalServerError;
};

class  Request{ // read event.
    std::string RawRequest;
    std::string type;
    std::string qkey;
    std::string qvalue;
    subState    SubState;
    char        httpGreater;
    char        httpMinor;
    int totalReadBytes;
    int _bytesread;
    bool keep_alive;
    std::string fullpath;
    // post request
    int Postfd;
    int totLent;
    std::string targetUri;
    std::map<std::string, std::string> queries;
    std::map<std::string, std::string> headers;
    Request(Request& rhs, int bytesread);
    Request& operator=(Request& rhs);
    std::map<int, std::string> parse_map;
    
    public:
    t_FILE  RequestFile;
    bool    hasMaxBodySize;
    size_t maxBodySize;
    bool openRequestFile;
    struct requestErrors _requestErrors;
    mainState MainState;
    Request();
    std::string getHttpVersion();
    void    setUpPostFile();
    void addtoheaders(std::string& key, std::string& val);
    std::string getRequestPath();
    const std::string& getRawRequest() const;
    const std::string& getType() const;
    void parseRequestHeader(char* request, int readBytes, std::vector<Location>);
    void parseRequestLine(char *request, int readBytes, int &offset);
    void parseRequestBody(char *, int , int, std::vector<Location>);
    std::string getMapAtIndex(unsigned int index);
    void printRequestLine();
    void printHeaderFields();
    void setConnectionType();
    bool isAlive();
    void reset();
    int getState();
    void    contentLengthBody(char *request, int offset, int readBytes);
    void    chunkedBody(char *, int, int);
    int     getPostFd();
    std::string getExtension();
    ~Request(){}
    bool    isValidPostPath(std::vector<Location> _locations);
    std::string getfullpath();
    std::map<std::string, std::string>& getHeaders();

    // cookies
    std::map<std::string, std::string> cookiesMap;
    void parseCookies(const std::string& cookieHeader);
    // 
    bool isvalidUploadPath(std::vector<Location> &locations);
    bool isDirectoryWritable(const char* dirPath);
    std::string generateUniqueFilename();
};
