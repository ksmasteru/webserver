#pragma once
#include <string>
#include <map>
#pragma once
#include <iostream>
#include <fcntl.h>
#include "../includes/utils.hpp"
#include "Location.hpp"
#include "utils.hpp"

class Location;
enum Chunk_State{
    chunk_size,
    CR1,
    LF1,
    chunk_data,
    CR2,
    write_chunk, // next time you write credit.
    CR3,
    LF3,
    chunk_done
};

    /*
        7\r\n
        Mozilla\r\n
        11\r\n
        Developer Network\r\n
        0\r\n
        \r\n
    */

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


    // cookies
    std::map<std::string, std::string> cookiesMap;
    void parseCookies(std::string cookies);

    void reset(){
        std::cout << "reset request ..." << std::endl;
        RawRequest.clear();
        type.clear();
        qkey.clear();
        qvalue.clear();
        RequestFile.fileName.clear();
        MainState = ReadingRequestHeader;
        SubState = start;
        totalReadBytes = 0;
        _bytesread = 0;
        openRequestFile = false;
        RequestFile.fd = -1; 
        _requestErrors.notAllowed = false;
        _requestErrors.badRequest = false;
    }
    int getState(){
        return this->MainState;
    }
    void    contentLengthBody(char *request, int offset, int readBytes);
    void    chunkedBody(char *, int, int);
    int     getPostFd();
    std::string getExtension();
    ~Request(){}
    bool    isValidPostPath(std::vector<Location> _locations);
    std::string getfullpath();
    const std::map<std::string, std::string>& getHeaders() const;
};
