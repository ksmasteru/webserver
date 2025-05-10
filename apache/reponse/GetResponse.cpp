#include "../includes/GetResponse.hpp"
#include <fstream>
#include <sys/stat.h>


GetResponse::GetResponse(const std::string& type, Request* req, std::map<std::string, 
std::string>*status, int client_fd) : AResponse(type, req, status, client_fd)
{
    // first checks if the file exist based on this info : fill body header
    // shalow copy of request.
}

// response header should be last to get filled.

std::string GetResponse::RspStatusline(unsigned int code)
{
    std::string statusCode = intToString(code);
    std::string Response;
    //403: Forbidden
    switch (code)
    {
        case 200:
            Response =  "OK";
            break;
        case 403:
            Response = "Forbidden";
            break;
        case 404:
            Response = "Not Found";
            break;
        default:
            Response = "Not allowed"; 
            break;
    }
    /*std::map<std::string, std::string>::iterator it;
    if ((it = statuscodes->find(statusCode)) != statuscodes->end())
        Response = it->second;*/
    std::string rsp = this->_request->getHttpVersion() + " " + statusCode + " " + Response + "\r\n";
    return (rsp);
}

std::string GetResponse::getTime()
{
    std::time_t now = std::time(nullptr);
    std::tm *gmt = std::gmtime(&now);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmt);
    return (buffer);
    
}

std::string GetResponse::RspHeader(unsigned int cLength, unsigned int code)
{
    std::string alive = "keep-alive"; // will be set later;
    std::ostringstream header;
    header  << RspStatusline(code)
            << "Date: " + getTime() + " \r\n"
            << "Server: apache/2.4.41 (Ubuntu) \r\n"
            << "Content-Type: " + this->res_data.contentType + " \r\n"
            //<< "Transfer-Encoding: chunked \r\n"
            << "Content-Length: " + intToString(cLength) + " \r\n"
            << "Connection: " + alive + " \r\n"
            << "\r\n";
    std::string head_msg = header.str();
    this->res_data.totallength = cLength + head_msg.length();
    return (head_msg);
}

std::string content(std::string extension )
{
   std::map<std::string, std::string> contentMap = {
        {"html", "text/html"},
        {"htm",  "text/html"},
        {"ico",  "image/png"},
        {"css",  "text/css"},
        {"js",   "application/javascript"},
        {"json", "application/json"},
        {"png",  "image/png"},
        {"jpg",  "image/jpg"},
        {"jpeg", "image/jpeg"},
        {"gif",  "image/gif"},
        {"svg",  "image/svg+xml"},
        {"txt",  "text/plain"},
        {"mp4", "video/mp4"},
        {"", "text/plain"}
    };
    return contentMap[extension];
}

void GetResponse::sendPage(const char *path, int cfd, bool redirection)
{
    //fill response detail  : extension, type, length.
    int fd =  open(path, O_RDONLY);
    if (fd == -1)
        throw ("couldnt open file");
    size_t extension_pos = this->_request->getRequestPath().rfind(".");
    if (redirection)
        this->res_data.extension = "html";
    else
        this->res_data.extension = (extension_pos != std::string::npos) ? this->_request->getRequestPath().substr(extension_pos+1) : "";
    this->res_data.contentType = content(this->res_data.extension);
    std::cout << "for path " << path << " content type is " << this->res_data.contentType << " extension is " << this->res_data.extension << std::endl;
    struct stat st;
    fstat(fd ,&st);
    this->res_data.clength = st.st_size;
    std::cout << "content length is " << this->res_data.clength << std::endl;
    std::string header = RspHeader(this->res_data.clength, this->res_data.status);
    // send header
    send(cfd, header.c_str(), header.length(), 0);
    //read and send page body.
    #define BUFF 1000
    char* buffer[BUFF];
    int bytesTosend = this->res_data.clength;
    int readbytes = 0;
    int total = 0;
    while (bytesTosend > 0)
    {
        readbytes = read(fd, buffer, BUFF);
        if (readbytes == -1)
        {
            std::cout << "read fail" << std::endl;
            break;
        }
        //dprintf(cfd, "%zx\r\n", readbytes);
        total += send(cfd, buffer, readbytes, MSG_NOSIGNAL);
        std::cout << "sent total of " << total << std::endl;
        //write(cfd, "\r\n", 2);
        bytesTosend -= readbytes;
    }
    //write(cfd, "0\r\n\r\n", 5);
    std::cout << "remaining bytes to send " << bytesTosend << std::endl;
    std::cout << "total sent bytes " << total << std::endl;
    close (fd);
    //close (cfd);
}

void GetResponse::handleErrorPage(const char *path, int cfd)
{
    if (access(path, F_OK) == -1)
    {
        this->res_data.status = 404;
        return (sendPage("pages/404.html", cfd, true));
    }
    else if (access(path, R_OK) == -1)
    {
        this->res_data.status = 403;
        return (sendPage("pages/403.html", cfd, true));
    }
}

// send repsonse and close cfd
void GetResponse::makeResponse(int cfd)
{
    std::string path = "pages" + this->_request->getRequestPath();
    this->res_data.status = 200;
    if (path.compare("pages/") == 0)
    {
        std::cout << "index page" << std::endl;
        sendPage("pages/index.html", cfd, true);
    }
    else if (access(path.c_str(),R_OK) == 0)
        return (sendPage(path.c_str(), cfd, false));
    else
        return (handleErrorPage(path.c_str(), cfd));    
}
const char* GetResponse::getRes() const
{
    return (this->resp_msg);
}

bool GetResponse::isAlive() const
{
    return true;
}


GetResponse::~GetResponse()
{
}

size_t  GetResponse::getSize()
{
    return (this->res_data.totallength);
}

