
#include "../includes/Response.hpp"
#include <fstream>
#include <sys/stat.h>

Response::Response()
{
    fileOffset = 0;
    sentBytes = 0;
    openfile = false;
    chunked = false;
    this->state = sendingheader;
    this->path_set = false;
    this->settings_set = false;
}

Response::Response(const std::string& type, Request* req, std::map<std::string, 
std::string>*status, int client_fd) : AResponse(type, req, status, client_fd)
{
    // first checks if the file exist based on this info : fill body header
    // shalow copy of request.
    fileOffset = 0;
    sentBytes = 0;
    openfile = false;
    chunked = false;
    this->state = sendingheader;
    this->path_set = false;
    this->settings_set = false;
}

// response header should be lsat to get filled.

std::string Response::RspStatusline(unsigned int code)
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
        case 301:
            Response = "Moved Permanently";
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

std::string Response::getTime()
{
    std::time_t now = std::time(nullptr);
    std::tm *gmt = std::gmtime(&now);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmt);
    return (buffer);
    
}

std::string Response::RspHeader(long long cLength, unsigned int code)
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
            << "Content-Length: " + longlongToString(cLength) + " \r\n"
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

void Response::sendHeader(const char *path, int cfd, bool redirection)
{
    //size_t extension_pos = this->_request->getRequestPath().rfind(".");// changed.
    std::string pathStr = this->getPath();
    size_t extension_pos = pathStr.rfind(".");
    if (redirection) // deprecated
        this->res_data.extension = "html";
    else
        this->res_data.extension = (extension_pos != std::string::npos) ? pathStr.substr(extension_pos+1) : "";
    this->res_data.contentType = content(this->res_data.extension);
    std::cout << "for path " << path << " content type is " << this->res_data.contentType << " extension is " << this->res_data.extension << std::endl;
    struct stat st;
    stat(path ,&st);
    this->res_data.clength = st.st_size; // ! overflow
    //std::cout << "content length is " << this->res_data.clength << std::endl;
    if (this->res_data.clength > INT16_MAX)
        chunked = true;
    else
        chunked = false;
    std::string header = RspHeader(this->res_data.clength, this->res_data.status);
    int sent = send(cfd, header.c_str(), header.length(), MSG_NOSIGNAL);
    // header is guaranted to be less than buffer size.
    if (sent == -1)
    {
        this->state = ResponseDone;
        close(this->fd);
        openfile = false;
        throw (cfd);
    }
    sentBytes += sent;
    //std::cout << "sent " << sent << std::endl;
    // setting new stat for the response
    this->state = sendingBody;
}

void Response::getFileReady(int fd)
{
    off_t new_offset = lseek(fd, fileOffset, SEEK_SET);
    if (new_offset == -1)
        throw ("bad file seek");
}

void Response::sendChunkHeader (int cfd, int readBytes)
{
    std::stringstream ss;
    ss << std::hex << readBytes << "\r\n";  // Convert to hexadecimal
    std::string hexStr = ss.str();
    //std::cout << "chunked header " << hexStr << "size of header" << hexStr.length() <<std::endl;
    if (send(cfd, hexStr.c_str(), hexStr.length(), MSG_NOSIGNAL) == -1)
    {
        close (this->fd);
        openfile = false;
        this->state = ResponseDone;
        std::cout << "closing connection on socket " << cfd << std::endl;
        throw (cfd);
    }
}

int Response::getFd(const char *path)
{
    if (openfile)
    return fd;
    this->fd = open(path, O_RDONLY);
    if (this->fd == -1)
        throw ("couldnt open file");
    openfile = true;
    return this->fd;
}

// when sending a video opt for chunked simply because it is hard to
// store its content lenght in a variable --> overflow == closedCOnnection
void Response::sendPage(const char *path, int cfd, bool redirection)
{
    // you could at the start open the file and keep it open
    // this way you dont have to lseek or multiple open close.
    // you only close after timeout or response all sent.
    std::cout << "send page is called with path " << path << std::endl;
    sentBytes = 0;
    if (this->getState() == sendingheader)
    {
        //std::cout << "sending header" << std::endl;
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
    //std::cout <<  "readbytes are " << readbytes << std::endl;
    if (chunked)
        sendChunkHeader(cfd, readbytes);
    int sent = send(cfd,  buffer, readbytes, MSG_NOSIGNAL);
    if (sent == -1)
    {
        close (fd);
        openfile = false;
        this->state = ResponseDone;
        throw ("send fail 1");
    }
    fileOffset += sent;
    if (chunked)
        if (send(cfd, "\r\n", 2, MSG_NOSIGNAL) == -1)
        {
            std::cout << "closing connection on " << cfd << std::endl;
            this->openfile = false;
            this->state = ResponseDone;
            close (fd);
            throw (cfd);
        } 
    if (readbytes < R_BUFF || readbytes == 0)
    {
        this->state = ResponseDone;
        close(fd);
        if (chunked)
            send(cfd, "0\r\n\r\n", 5, 0);
    }
    // reset timer of clients.
    //std::cout << "sent " << sent << " file offset is " <<  fileOffset << std::endl;
}

/*to avoid client closing the connection ; which
will result  in a sigpipe sending the error page
in one send and thus setting response status to Done*/
void Response::sendNotFoundPage(const char* path, int cfd, bool redir)
{
    std::cout << "sending not found page" << std::endl;
    // try sending header then send body.
    // if it doesnt work send both header and body in one.
    sendHeader(path, cfd, redir);
    int R_BUFF = this->res_data.clength;
    int fd =  getFd(path);
    char *buffer[R_BUFF];
    int readBytes = read(fd, buffer, R_BUFF);
    if (readBytes < 0)
        throw ("read fail");
    std::cout <<  "readbytes are " << readBytes << std::endl;
    int sent = send(cfd,  buffer, readBytes, MSG_NOSIGNAL);
    if (sent != readBytes)
    {
        close (fd);
        openfile = false;
        throw ("send fail 1");
    }
    close(fd);
    this->state = ResponseDone;
}

void Response::handleErrorPage(const char *path, int cfd)
{
    if (access(path, F_OK) == -1)
    {
        this->res_data.status = 404;
        return (sendNotFoundPage("pages/404.html", cfd, true));
    }
    else if (access(path, R_OK) == -1)
    {
        this->res_data.status = 403;
        return (sendPage("pages/403.html", cfd, true));
    }
}

// returns value form start to /
// return empty string in failure.
// /images/ronaldo.png returns /images and set filename to ronaldo.png.
std::string Response::getFolderName(const std::string& path)
{
    // skip first /
    size_t pos = path.find('/', 1);
    if (pos == std::string::npos) //  wasnot found
    {
        /*redirected = true;
        if (path != "/")
            this->res_data.status = 301;*/
        return (path);
    }
    return (path.substr(0, pos)); // here  the  is actually found so no redirection.
}

// /images/hello.png --> hello.png.
std::string getFileName(std::string path, std::string folderName)
{
    if (folderName == path || folderName + "/" == path)
    {
        std::cout << "file name is empty first condition" << std::endl;
        return ("");
    }
    return (path.substr(folderName.size()));
}

void Response::setResponseSettings(Location& _location, int index)
{
    std::vector<std::string> methods = _location.getAllowedMethods();
    this->settings.Locationindex = index;
    for (size_t i = 0; i < methods.size(); i++)
    {
        if (methods[i] == "GET")
            this->settings.GET = true;
        else if (methods[i] == "POST")
            this->settings.POST = true;
        else if (methods[i] == "DELETE")
            this->settings.DELETE = true;
    }
    this->settings_set = true;
}
std::string Response::getPagePath2(std::string path, std::vector<Location>& locations)
{
    this->path_set = true;
    this->res_data.status = 200; // initial;
    std::cout << "provided path is " << path << std::endl;
    std::string folderName = getFolderName(path);
    std::cout << "folderName is : " << folderName << std::endl;
    std::string fileName = getFileName(path, folderName);
    std::cout << "filename is : " << fileName << std::endl;
    if (path != "/")
    {
        for (size_t i = 0; i < locations.size(); i++)
        {
            std::cout << "location[i] path is : " << locations[i].getPath() << std::endl;
            if (locations[i].getPath() != "/" && locations[i].getPath() + "/" == path)
            {
                std::cout << "First condition" << std::endl;
                std::cout << "to return " << locations[i].getIndex() << std::endl;
                //exit(1);
                setResponseSettings(locations[i], i);
                return(locations[i].getIndex());
            }
            else if (locations[i].getPath() == path && locations[i].getPath() != "/")
            {
                std::cout << "Second condition : redirection" << std::endl;
                std::cout << "to return : " << locations[i].getPath() + "/" << std::endl;
                //exit(1);
                setResponseSettings(locations[i], i);
                this->res_data.status = 301;
                this->settings.redirected = true;
                return (locations[i].getPath() + "/"); // images/
            }
            else if (locations[i].getPath() == folderName)
            {
                std::cout << "second condition" << std::endl;
                std::cout << "to return : " << locations[i].getRoot() + fileName << std::endl;
                //exit(1);
                setResponseSettings(locations[i], i);
                return (locations[i].getRoot() + fileName);
            }
        }
    }
    /*if no matching location use root.*/
    for (size_t i = 0; i < locations.size(); i++)
    {
        if (locations[i].getPath() == "/")
        {
            //std::cout << "third condition" << std::endl;
            setResponseSettings(locations[i], i);
            if (path == "/")
                return (locations[i].getIndex());
            return (locations[i].getRoot() + path);
        }
    }
    //std::cout << "fourth condition" << std::endl;
    return ("");
}
/*
void Response::notAllowedResponse(int cfd)
{
    std::string allowedMethods = "Allow: ";
    if (this->setting.GET)
        allowedMethods += "GET, ";
    if (this->settings.POST)
        allowedMethods += "POST ,";
    if (this->settings.DELETE)
        allowedMethods += "DELETE";
    std::ostringstream msg;
    msg << "HTTP/1.1 405 Method Not Allowed \r\n"
        << "Date: " + getTime() + " \r\n"
        << "Server: apache/2.4.41 (mac osx) \r\n"
        <<  "Content-Length: 0 \r\n"
        << allowedMethods + " \r\n"
        << "\r\n";
    std::string resp = msg.str();
    if (send(cfd, resp.c_str(), resp.size(), MSG_NOSIGNAL) == -1)
    {
        std::cout << " failed to send " << cfd << std::endl;
        this->state = ResponseDone;
        throw (cfd);
    }
    this->state = ResponseDone;
}*/

void Response::notAllowedGetResponse(int cfd) // returns status ccoddee of 405
{
    std::string allowedMethods = "Allow: ";
    if (this->settings.POST)
        allowedMethods += "POST ,";
    if (this->settings.DELETE)
        allowedMethods += "DELETE";
    std::ostringstream msg;
    /*HTTP/1.1 405 Method Not Allowed
    Content-Length: 0
    Date: Fri, 28 Jun 2024 14:30:31 GMT
    Server: ECLF (nyd/D179)
    Allow: GET, POST, HEAD*/
    msg << "HTTP/1.1 405 Method Not Allowed \r\n"
        << "Date: " + getTime() + " \r\n"
        << "Server: apache/2.4.41 (mac osx) \r\n"
        <<  "Content-Length: 0 \r\n" 
        << allowedMethods + " \r\n"
        << "\r\n";
    std::string resp = msg.str();
    if (send(cfd, resp.c_str(), resp.size(), MSG_NOSIGNAL) == -1)
    {
        std::cout << " failed to send " << cfd << std::endl;
        this->state = ResponseDone;
        throw (cfd);
    }
    this->state = ResponseDone;
}

void Response::notFoundResponsePage(std::map<int, std::string>& errorPages)
{
    // EACH  CONFIG file should have a not found page.
    // handle if an errorpage wasnt assigned in the config file
    // if (errorPages.find(404) == errorPages.end())
    std::string pagePath = errorPages[404];
    // redirection set to true;
}

// change name to forbidenResponsePage. // should be a general errorPage.
/*void Response::accessDeniedResponsePage(std::map<int, std::string>& errorPages)
{
    std::cout << "Access denied response page called" << std::endl;
    std::string path;
    std::map<int, std::string>::iterator it;
    it = errorPages.find(403);
    if (it != errorPages.end())
    {
        path = errorPages[403];
        if (access(path.c_str(), F_OK) == -1 || access(path.c_str(), R_OK) == -1)
            path = "./pages.403.html";
    }
    else
        path = "./pages/403.html";
    sendHeader(path, cfd, true);
    int R_BUFF = this->res_data.clength;
    int fd =  getFd(path.c_str());
    char *buffer[R_BUFF];
    int readBytes = read(fd, buffer, R_BUFF);
    if (readBytes < 0)
        throw ("read fail");
    std::cout <<  "readbytes are " << readBytes << std::endl;
    int sent = send(cfd,  buffer, readBytes, MSG_NOSIGNAL);
    if (sent != readBytes)
    {
        close (fd);
        openfile = false;
        this->state = ResponseDone;
        throw ("send fail 1");
    }
    close(fd);
    this->state = ResponseDone;
}*/

// in location "provided by config file" if path is exact match of location's path
// return location index attribute (if set) : in  this example it is always set
// example : path = /pages ; returns index.html.
bool Response::handlePathRedirection(std::string path, std::vector<Location>& locations)
{
    int location_index = this->settings.Locationindex;
    return true;
}

void Response::errorResponsePage(int cfd, std::map<int, std::string>& errorPages, int errorCode)
{
    std::cout << "Error reponse page: " << cfd << std::endl;
    std::map<int, std::string>::iterator it;
    std::string path;
    it = errorPages.find(403);
    if (it != errorPages.end())
    {
        path = errorPages[403];
        if ((access(path.c_str(), F_OK) == -1) || (access(path.c_str(), R_OK) == -1))
            path = "./pages/403.html";
    }
    else
    {
        switch (errorCode)
        {
            case 403:
                path = "./pages/403.html";
                break;
            case 404:
                path = "./pages/404.html";
                break;
            default:
                std::cerr << "Unset error page: " << errorCode << " not handled yet!" << std::endl;
                exit(1);
        }
        if (access(path.c_str(), R_OK) == -1)
        {
            std::cout << "missing default error page " << errorCode << std::endl;
            this->state = ResponseDone;
            throw(cfd);
        }
    }
    sendHeader(path.c_str(), cfd, true);
    int R_BUFF = this->res_data.clength;
    int fd = getFd(path.c_str());
    char *buffer[R_BUFF];
    int readBytes = read(fd, buffer, R_BUFF);
    if (readBytes < 0)
        throw ("read fail");
    std::cout << "readBytes are " << readBytes << std::endl;
    int sent = send (cfd, buffer, readBytes, MSG_NOSIGNAL);
    if (sent != readBytes)
    {
        close(fd);
        openfile = false;
        this->state = ResponseDone;
        throw("send fail on error page");
        // should here also throw an error. ?
    }
    close(fd);
    this->state = ResponseDone;
}


void Response::redirectResponse(int cfd, const char *path)
{
    //  send a redirect 301 with
    std::cout << "----------redirectResponse called--------" << std::endl;
    this->settings.redirected = false;
    std::ostringstream ofs;
    std::string location = "location: ";
    ofs << "HTTP/1.1 301 Moved Permanently \r\n"
        << location + path + " \r\n"
        << "Connection: close \r\n"
        << "\r\n";
    
    std::string resp = ofs.str();
    if (send(cfd, resp.c_str(), resp.size(), MSG_NOSIGNAL) == -1)
    {
        std::cout << "Send fail at redirectResponse" << std::endl;
        this->state = ResponseDone;
        throw (cfd);
    }
    this->state = ResponseDone;
}
// ?? something missing
// send repsonse and close cfd for GET!!!.
// this response if for GET REQUEST ONLLY IF IN CONFIG FILE A PATH ISNT ALLOWED GET RETURNS 405 Method Not Allowed
// web
void Response::makeResponse(int cfd, Request* req, std::map<int, std::string> &errorPages,
    std::vector<Location> &locations)
{
    //  will add a new response step called pathSet. or just a flag.
    this->_request = req;
    this->settings.redirected = false;
    std::cout << "make response called" << std::endl;
    if (!this->path_set)
    {
        this->filePath = getPagePath2(req->getRequestPath(), locations);
        std::cout << "file path is: " << this->filePath << std::endl;
        this->path_set = true;
        if (!this->settings.GET)
            return (notAllowedGetResponse(cfd));
        if (this->settings.redirected == true)
            return (redirectResponse(cfd, this->filePath.c_str()));
        // checking if the path actualy exist and allowed to be accessed.
        if (access(this->filePath.c_str(), F_OK) == 0)
        {
            if (access(this->filePath.c_str(), R_OK) == -1)
            {
                this->res_data.status = 403;
                return (errorResponsePage(cfd, errorPages, 403));
            }
        }
        else /*page not found*/
        {
            this->res_data.status = 404;
            return (errorResponsePage(cfd, errorPages, 404));
        }
    }
    //bool redirection = handlePathRedirection(path, locations); // ?
    // if path is empty it should change it to index here ?
    //std::string path = "pages" + req->getRequestPath();
    //this->res_data.status = 200;
    /*
    if (path.compare("pages/") == 0)
    {
        //std::cout << "index page" << std::endl;
        sendPage("pages/index.html", cfd, true);
    }
    else if (access(path.c_str(), R_OK) == 0)
        return (sendPage(path.c_str(), cfd, false));
    else
        return (handleErrorPage(path.c_str(), cfd));*/
    // new logic.
    // get page Path should be reponsible for  index and redirection.
    sendPage(this->filePath.c_str(), cfd, false);
}

void Response::handleBadRequest(int cfd, Request *req)
{
    std::ostringstream ofs;
    if (req->_requestErrors.badRequest)
    {
        ofs << "HTTP/1.1 400 Bad Request \r\n"
            << "\r\n";
    }
    else if (req->_requestErrors.notAllowed)
    {
        ofs << "HTTP/1.1 405 Method Not Allowed \r\n"
            << "Server: apache/2.4.41 (mac osx) \r\n"
            << "Content-Length: 0 \r\n"
            << "\r\n";
    }
    std::string resp = ofs.str();
    if (send(cfd, resp.c_str(), resp.length(), MSG_NOSIGNAL) == -1)
    {
        std::cout << "send error in handle bad Request" << std::endl;
        throw (1);
    }
    this->state = ResponseDone;
}

// header of a successful post response.
void Response::successPostResponse(int cfd)
{
    std::ostringstream ofs;
    ofs << "HTTP/1.1 201 Created \r\n"
        << "Connection: Close \r\n"
        << "\r\n";
    std::string header = ofs.str();
    if (send(cfd, header.c_str(), header.length(), MSG_NOSIGNAL) == -1)
        throw ("error sending post Response");
    std::cout << "sent post response" << header <<std::endl;
    this->state = ResponseDone;
}

const char* Response::getRes() const
{
    return (this->resp_msg);
}

bool Response::isAlive() const
{
    return true;
}

Response::~Response()
{
}

size_t  Response::getSize()
{
    return (this->res_data.totallength);
}

std::string Response::getPath()
{
    return (this->filePath);
}

void Response::deleteResponse(int cfd, Request* req)
{
    // DELETE /file.html HTTP/1.1
    // the request object handle the deletion and update code . state. 
    this->_request = req; // ?
    std::ostringstream ofs;
    std::string path = "pages/" + req->getRequestPath();
    if (access(path.c_str(), F_OK) == -1)
        this->res_data.status = 404;
    else if (access(path.c_str(), W_OK) == -1)
        this->res_data.status = 403; // forbidden.
    else
    {
        if (unlink(path.c_str()) == -1) /*internal error*/
            this->res_data.status = 403;
        else
            this->res_data.status = 204;
    }
    switch (this->res_data.status)
    {
        case 204:
            ofs << "HTTP/1.1 204 No Content \r\n"
                << "Date: " + getTime() + " \r\n"
                << "\r\n";
            if (send(cfd, ofs.str().c_str(), ofs.str().length(), MSG_NOSIGNAL) == -1)
                throw(cfd);
            break;
        case 403:
            ofs << "HTTP/1.1 403 Forbidden \r\n"
                << "Date: " + getTime() + " \r\n"
                << "\r\n";
            if (send(cfd, ofs.str().c_str(), ofs.str().length(), MSG_NOSIGNAL) == -1)
                throw(cfd);
            break;
        case 404:
            ofs << "HTTP/1.1 404 Not Found \r\n"
                << "Date: " + getTime() + " \r\n" // needs additional \r\n ?
                << "\r\n";
            if (send(cfd, ofs.str().c_str(), ofs.str().length(), MSG_NOSIGNAL) == -1)
                throw(cfd);
            break;
        default:
            break;
    }
    this->state = ResponseDone;
}

void Response::sendTimedOutResponse(int cfd)
{
    std::ostringstream ofs;

    ofs << "HTTP/1.1 408 Request Timeout\r\n"
        << "Content-Length: 0\r\n"
        << "\r\n";
    
    std::string msg = ofs.str();
    if (send(cfd, msg.c_str(), msg.length() , MSG_NOSIGNAL) == -1)
        throw (cfd);
    this->state = ResponseDone;
}