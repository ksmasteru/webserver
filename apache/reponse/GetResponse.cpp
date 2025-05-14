#include "../includes/GetResponse.hpp"
#include <fstream>
#include <sys/stat.h>


GetResponse::GetResponse()
{
    fileOffset = 0;
    sentBytes = 0;
    openfile = false;
    chunked = false;
    this->state = sendingheader;
}

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
    if (!chunked)
    {
        header  << RspStatusline(code)
            << "Date: " + getTime() + " \r\n"
            << "Server: apache/2.4.41 (Ubuntu) \r\n"
            << "Content-Type: " + this->res_data.contentType + " \r\n"
            //<< "Transfer-Encoding: chunked \r\n"
            << "Content-Length: " + intToString(cLength) + " \r\n"
            << "Connection: " + alive + " \r\n"
            << "\r\n";
    }
    else
    {
            header  << RspStatusline(code)
            << "Date: " + getTime() + " \r\n"
            << "Server: apache/2.4.41 (Ubuntu) \r\n"
            << "Content-Type: " + this->res_data.contentType + " \r\n"
            << "Transfer-Encoding: chunked \r\n"
            << "Connection: " + alive + " \r\n"
            << "\r\n";
    }
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

void GetResponse::sendHeader(const char *path, int cfd, bool redirection)
{
    size_t extension_pos = this->_request->getRequestPath().rfind(".");
    if (redirection)
        this->res_data.extension = "html";
    else
        this->res_data.extension = (extension_pos != std::string::npos) ? this->_request->getRequestPath().substr(extension_pos+1) : "";
    this->res_data.contentType = content(this->res_data.extension);
    //std::cout << "for path " << path << " content type is " << this->res_data.contentType << " extension is " << this->res_data.extension << std::endl;
    struct stat st;
    stat(path ,&st);
    this->res_data.clength = st.st_size;
    //std::cout << "content length is " << this->res_data.clength << std::endl;
    std::string header = RspHeader(this->res_data.clength, this->res_data.status);
    int sent = send(cfd, header.c_str(), header.length(), MSG_NOSIGNAL);
    // header is guaranted to be less than buffer size.
    if (sent == -1)
        throw ("sending heade error");
    sentBytes += sent;
    std::cout << "sent " << sent << std::endl;
    // setting new stat for the response
    this->state = sendingBody;
}

void GetResponse::getFileReady(int fd)
{
    off_t new_offset = lseek(fd, fileOffset, SEEK_SET);
    if (new_offset == -1)
        throw ("bad file seek");
}
void GetResponse::sendChunkHeader (int cfd, int readBytes)
{
    std::stringstream ss;
    ss << std::hex << readBytes << "\r\n";  // Convert to hexadecimal
    std::string hexStr = ss.str();
    std::cout << "chunked header " << hexStr << "size of header" << hexStr.length() <<std::endl;
    if (send(cfd, hexStr.c_str(), hexStr.length(), 0) == -1)
        throw ("bad send");
}

int GetResponse::getFd(const char *path)
{
     if (openfile)
        return fd;
    this->fd = open(path, O_RDONLY);
    if (this->fd == -1)
        throw ("couldnt open file");
    openfile = true;
    return this->fd;
}
void GetResponse::sendPage(const char *path, int cfd, bool redirection)
{
    // you could at the start open the file and keep it open
    // this way you dont have to lseek or multiple open close.
    // you only close after timeout or response all sent.
    sentBytes = 0;
    chunked = true;
    if (this->getState() == sendingheader)
    {
        std::cout << "sending header" << std::endl;
        sendHeader(path, cfd, redirection);
    }
    int R_BUFF = BUFFER_SIZE - sentBytes;
    if (R_BUFF < 2)
        throw ("too long of a header?");
    int fd = getFd(path);
    char* buffer[R_BUFF];
    //getFileReady(fd);
    int readbytes = read(fd, buffer, R_BUFF);
    if (readbytes < 0)
        throw ("read fail");
    if (chunked)
        sendChunkHeader(cfd, readbytes);
    int sent = send(cfd,  buffer, readbytes, 0);
    if (sent == -1)
    {
        close (fd);
        throw ("send fail");
    }
    fileOffset += sent;
    if (chunked)
        send(cfd, "\r\n", 2, 0); 
    if (readbytes < R_BUFF || readbytes == 0)
    {
        this->state = done;
        if (chunked)
            send(cfd, "0\r\n\r\n", 5, 0);
    }
    std::cout << "sen " << sent << " file offset is " <<  fileOffset << std::endl;
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
void GetResponse::makeResponse(int cfd, Request* req)
{
    this->_request = req;
    std::cout << "make response called" << std::endl;
    std::string path = "pages" + req->getRequestPath();
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

