// flags done : execept c++98 for addCookiesHeader.
#include "../includes/Response.hpp"
#include "../includes/cgiHandler.hpp"
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include <sstream>

Response::Response()
{
    fileOffset = 0;
    sentBytes = 0;
    openfile = false;
    chunked = false;
    this->state = sendingheader;
    this->path_set = false;
    this->settings_set = false;
    this->settings.autoIndexed = false;
    this->settings.dirUrl = false;
    this->settings.indexFile = false;
}

Response::Response(const std::string& type, Request* req, std::map<std::string, 
std::string>*status, int client_fd) : AResponse(type, req, status, client_fd)
{
    fileOffset = 0;
    sentBytes = 0;
    openfile = false;
    chunked = false;
    this->state = sendingheader;
    this->path_set = false;
    this->settings_set = false;
    this->settings.dirUrl = false;
    this->settings.indexFile = false;
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
    std::time_t now = std::time(00);
    std::tm *gmt = std::gmtime(&now);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmt);
    return (buffer);
}

void Response::addCookiesHeader(std::ostringstream& ofs)
{
    std::map<std::string, std::string>::iterator it = this->_request->cookiesMap.begin();
    if (it == _request->cookiesMap.end())
    {
        std::time_t now_c = std::time(0);
        std::ostringstream timeStr;
        timeStr << now_c;
        
        std::ostringstream randStr;
        randStr << (rand() % 10000);
        std::string sessionId = timeStr.str() + randStr.str();
        ofs << "Set-Cookie: sessionId=" << sessionId << "; Path=/; HttpOnly\r\n";
    }
    else
    {
        while (it != _request->cookiesMap.end())
        {
            ofs << "Set-Cookie: " << it->first << "=" << it->second << "; Path=/; HttpOnly\r\n";
            ++it;
        }
    }
}

std::string Response::RspHeader(long long cLength, unsigned int code)
{
    std::string alive = "keep-alive"; // will be set later;
    std::ostringstream header;
    if (!chunked)
    {
        header << RspStatusline(code)
            << "Date: " + getTime() + " \r\n"
            << "Server: apache/2.4.41 (Ubuntu) \r\n"
            << "Content-Type: " + this->res_data.contentType + " \r\n"
            //<< "Transfer-Encoding: chunked \r\n"
            << "Content-Length: " + longlongToString(cLength) + " \r\n"
            << "Connection: " + alive + " \r\n";
    }
    else
    {
            header  << RspStatusline(code)
            << "Date: " + getTime() + " \r\n"
            << "Server: apache/2.4.41 (Ubuntu) \r\n"
            << "Content-Type: " + this->res_data.contentType + " \r\n"
            << "Transfer-Encoding: chunked \r\n"
            << "Connection: " + alive + " \r\n";
    }
    addCookiesHeader(header);
    header << "\r\n";
    std::string head_msg = header.str();
    this->res_data.totallength = cLength + head_msg.length();
    return (head_msg);
}

std::string content(std::string extension )
{
    std::map<std::string, std::string> contentMap;
    contentMap.insert(std::make_pair("html", "text/html"));
    contentMap.insert(std::make_pair("htm",  "text/html"));
    contentMap.insert(std::make_pair("ico",  "image/png"));
    contentMap.insert(std::make_pair("css",  "text/css"));
    contentMap.insert(std::make_pair("js",   "application/javascript"));
    contentMap.insert(std::make_pair("json", "application/json"));
    contentMap.insert(std::make_pair("png",  "image/png"));
    contentMap.insert(std::make_pair("jpg",  "image/jpg"));
    contentMap.insert(std::make_pair("jpeg", "image/jpeg"));
    contentMap.insert(std::make_pair("gif",  "image/gif"));
    contentMap.insert(std::make_pair("svg",  "image/svg+xml"));
    contentMap.insert(std::make_pair("txt",  "text/plain"));
    contentMap.insert(std::make_pair("mp4",  "video/mp4"));
    contentMap.insert(std::make_pair("",     "text/plain"));
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

void Response::setResponseSettings(Location& _location, int index, bool dirUrl)
{
    std::vector<std::string> methods = _location.getAllowedMethods();
    // new 8/8 : index file flag;
    if (access(_location.getIndex().c_str(), R_OK) != -1)
        this->settings.indexFile = true;
    this->settings.autoIndexed = _location.getAutoindex();
    this->settings.dirUrl = dirUrl;
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

// new handling if locations[i] is a redirect block.
std::string Response::getPagePath2(std::string path, std::vector<Location>& locations)
{
    this->path_set = true;
    this->res_data.status = 200; // initial;
    std::cout << "provided path is " << path << std::endl;
    std::string folderName = getFolderName(path);
    std::cout << "folderName is : " << folderName << std::endl;
    std::string fileName = getFileName(path, folderName);
    std::cout << "filename is : " << fileName << std::endl;
    std::map<int, std::string>::iterator it;
    if (path != "/")
    {
        for (size_t i = 0; i < locations.size(); i++)
        {
            std::cout << "location[i] path is : " << locations[i].getPath() << std::endl;
            if (locations[i].getPath() != "/" && locations[i].getPath() + "/" == path)
            {
                // new check if is a redirection.
                if (locations[i].redirectBlock)
                {
                    std::cout << "---- redirect block detected in first condition ----" << std::endl; 
                    it = locations[i].redirections.begin();
                    std::cout << "--- redirecting to " << (it)->second << std::endl;
                    this->settings.redirected = true;
                    //exit(1);
                    return (it->second);
                }
                std::cout << "First condition" << std::endl;
                std::cout << "to return " << locations[i].getIndex() << std::endl;
                setResponseSettings(locations[i], i, true);
                if (this->settings.indexFile)/*priority of index file 8/8*/
                    this->settings.autoIndexed = false;
                if (this->settings.autoIndexed)
                {
                    std::cout << "auto index detected ..." << std::endl;
                    //std::cout << "todo directory listing " << locations[i].getRoot() << std::endl;
                    //exit(1);
                    return (locations[i].getRoot());
                }
                return(locations[i].getIndex());
            }
            else if (locations[i].getPath() == path && locations[i].getPath() != "/") /* no folder name*/
            {
                if (locations[i].redirectBlock)
                {
                    std::cout << "--- redirect block detected in second condition---" << std::endl;
                    it = locations[i].redirections.begin();
                    std::cout << "--- redirecting to " << it->second << std::endl; // wrong.
                    //std::cout << "--- redirecting to " << (it)->second << std::endl; // wrong.
                    //exit(1);
                    this->settings.redirected = true;
                    return (it->second);
                }
                std::cout << "to return : " << locations[i].getPath() + "/" << std::endl;
                std::cout << "Second condition : redirection" << std::endl;
                setResponseSettings(locations[i], i, true);
                this->res_data.status = 301;
                this->settings.redirected = true;
                return (locations[i].getPath() + "/"); // images/
            }
            else if (locations[i].getPath() == folderName) // images/ronaldo.jpg.
            {
                if (locations[i].redirectBlock)
                {
                    std::cout << "--- redirect block detected ---" << std::endl;
                    it = locations[i].redirections.begin();
                    if (it->second == "/")
                        std::cout << "to return : " << fileName << std::endl;
                    else
                        std::cout << "to return : " << it->second + fileName << std::endl;
                    this->settings.redirected = true;
                    if (it->second ==  "/")
                        return (fileName);
                    return (it->second + fileName);
                }
                std::cout << "to return : " << locations[i].getRoot() + fileName << std::endl;
                std::cout << "third condition" << std::endl;
                setResponseSettings(locations[i], i, false);
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
            if (path == "/")
            {
                setResponseSettings(locations[i], i, true);
                return (locations[i].getIndex());
            }
            setResponseSettings(locations[i], i, true);
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
    /*
    HTTP/1.1 405 Method Not Allowed
    Content-Length: 0
    Date: Fri, 28 Jun 2024 14:30:31 GMT
    Server: ECLF (nyd/D179)
    Allow: GET, POST, HEAD
    */
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

// static std::string intToString(int value)
// {
//     std::ostringstream oss;
//     oss << value;
//     return oss.str();
// }

void sendSimpleErrorPage(int cfd, int errorCode)
{
    std::string statusText;

    switch (errorCode)
    {
        case 400: statusText = "Bad Request"; break;
        case 401: statusText = "Unauthorized"; break;
        case 403: statusText = "Forbidden"; break;
        case 404: statusText = "Not Found"; break;
        case 405: statusText = "Method Not Allowed"; break;
        case 500: statusText = "Internal Server Error"; break;
        default:  statusText = "Error"; break;
    }

    std::string codeStr = intToString(errorCode);

    // HTML body
    std::string body =
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head><meta charset=\"UTF-8\"><title>" + codeStr + " " + statusText + "</title></head>\n"
        "<body style=\"font-family: Arial; text-align: center; margin-top: 10%;\">\n"
        "<h1>" + codeStr + " - " + statusText + "</h1>\n"
        "<p>Generated by WebServer</p>\n"
        "</body>\n"
        "</html>\n";

    // HTTP response
    std::string response =
        "HTTP/1.1 " + codeStr + " " + statusText + "\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: " + intToString(body.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" +
        body;

    // send to client
    if (send(cfd, response.c_str(), response.size(), MSG_NOSIGNAL) == -1)
    {
        // error handling
        std::cout << "send failed on sendSimpleErrorPage" << std::endl;
        throw (cfd);
    }
}

// fix error page from static to dynamic from config file
void Response::errorResponsePage(int cfd, std::map<int, std::string>& errorPages, int errorCode)
{
    std::cout << "Error reponse page: " << cfd << std::endl;
    std::map<int, std::string>::iterator it;
    std::string path;
    // it = errorPages.find(403);
    // if (it != errorPages.end())
    // {
        // path = errorPages[403];
        // if ((access(path.c_str(), F_OK) == -1) || (access(path.c_str(), R_OK) == -1))
        //     path = "./pages/403.html";
    // }
    // else
    // {
        // switch (errorCode)
        // {
        //     case 403:
        //         path = errorPages[403];
        //         // if ((access(path.c_str(), F_OK) == -1) || (access(path.c_str(), R_OK) == -1))
        //         //     path = "./pages/403.html";
        //         break;
        //     case 404:
        //         path = errorPages[404];
        //         break;
        //     default:
        //         std::cerr << "Unset error page: " << errorCode << " not handled yet!" << std::endl;
        //         exit(1);
        //         // exit ? should be removed
        // }
        std::cout << "\n\nerrorCode\n\n : \n" << errorCode << "\n";
        path = errorPages[errorCode];
        if (access(path.c_str(), R_OK) == -1)
        {
            std::cout << "missing default error page " << errorCode << std::endl;
            this->state = ResponseDone;
            openfile = false;
            sendSimpleErrorPage(cfd, errorCode);
            return;
            // throw(cfd);
            // instead of throwing send a simple html response as Code
        }
    // }
    sendHeader(path.c_str(), cfd, true);
    int R_BUFF = this->res_data.clength;
    int fd = getFd(path.c_str());
    char *buffer[R_BUFF];
    int readBytes = read(fd, buffer, R_BUFF);
    if (readBytes < 0)
        throw ("read fail");
    std::cout << "readBytes are " << readBytes << std::endl;
    int sent = send(cfd, buffer, readBytes, MSG_NOSIGNAL);
    if (sent != readBytes)
    {
        close(fd);
        openfile = false;
        this->state = ResponseDone;
        throw (cfd);
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
void Response::handleCgiRequest(const std::string &scriptPath, int cfd, Request *req, std::map<int, std::string>& errorPages)
{
    try
    {
        if (access(scriptPath.c_str(), X_OK) != 0)
        {
            std::cout << "CGI script not executable: " << scriptPath << std::endl;
            this->res_data.status = 403;
            //return handleErrorPage(scriptPath.c_str(), cfd);
            return (errorResponsePage(cfd, errorPages, 403));
        }

        Cgi cgiHandler(req, this, scriptPath, 5);

        std::cout << "Executing CGI script: " << scriptPath << std::endl;

        cgiHandler.execute_cgi();

        cgiHandler.handle_cgi_output();

        if (this->isCgiResponse())
        {
            std::cout << "CGI execution successful, sending response" << std::endl;
            sendCgiResponse(cfd);
        }
        else
        {
            std::cout << "CGI execution failed, no output received" << std::endl;
            this->res_data.status = 500;
            handleErrorPage(scriptPath.c_str(), cfd);
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "CGI execution error: " << e.what() << std::endl;

        std::string error_msg = e.what();

        if (error_msg.find("timeout") != std::string::npos)
        {
            // Prepare the 504 Gateway Timeout response
            std::string body =
                "<html>\r\n"
                "<head><title>504 Gateway Timeout</title></head>\r\n"
                "<body>\r\n"
                "<h1>504 Gateway Timeout</h1>\r\n"
                "<p>The CGI script took too long to respond.</p>\r\n"
                "</body>\r\n"
                "</html>\r\n";

            std::ostringstream header;
            header << 
                "HTTP/1.1 504 Gateway Timeout\r\n"
                << "Content-Type: text/html\r\n"
                << "Content-Length: "
                << longlongToString(body.length()) + "\r\n"
                                                << "Connection: close\r\n";
                addCookiesHeader(header);
            header << "\r\n";
            std::string response = header.str() + body;
            // Send the response to the client
            send(cfd, response.c_str(), response.length(), 0);
            close(cfd);
            this->res_data.status = 504;
            this->resetCgiData();
            return;
            // Close the connection after sending the response
        }
        else if (error_msg.find("not found") != std::string::npos ||
                 error_msg.find("No such file") != std::string::npos)
        {
            this->res_data.status = 404;
        }
        else if (error_msg.find("Permission denied") != std::string::npos)
        {
            this->res_data.status = 403;
        }
        else
        {
            this->res_data.status = 500;
        }

        handleErrorPage(scriptPath.c_str(), cfd);
    }
}


void Response::sendCgiResponse(int cfd)
{
    std::string httpResponse = this->buildCgiResponse();
    if (httpResponse.empty())
    {
        std::cout << "Empty CGI response, sending 500 error" << std::endl;
        this->res_data.status = 500;
        return handleErrorPage("", cfd);
    }

    ssize_t totalSent = 0;
    ssize_t responseSize = httpResponse.length();
    const char *responseData = httpResponse.c_str();

    std::cout << "Sending CGI response (" << responseSize << " bytes)" << std::endl;

    int error = 0;
    socklen_t len = sizeof(error);
    if (getsockopt(cfd, SOL_SOCKET, SO_ERROR, &error, &len) != 0 || error != 0)
    {
        std::cout << "Connection already closed, aborting CGI response send" << std::endl;
        return;
    }

    while (totalSent < responseSize)
    {
        ssize_t sent = send(cfd, responseData + totalSent,
                            std::min((ssize_t)8192, responseSize - totalSent),
                            MSG_NOSIGNAL);

        if (sent == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                fd_set writefds;
                FD_ZERO(&writefds);
                FD_SET(cfd, &writefds);
            }
            else if (errno == EPIPE || errno == ECONNRESET)
            {
                std::cout << "Client disconnected (broken pipe/connection reset)" << std::endl;
                break;
            }
            else
            {
                std::cout << "Error sending CGI response: " << strerror(errno) << std::endl;
                break;
            }
        }
        else if (sent == 0)
        {
            std::cout << "Client disconnected during CGI response send" << std::endl;
            break;
        }

        totalSent += sent;
        std::cout << "Sent " << sent << " bytes (" << totalSent << "/" << responseSize << ")" << std::endl;
    }

    this->_progress.sentBytes = totalSent;
    this->_progress.sentTotal = responseSize;
    this->_progress.progress = (totalSent == responseSize) ? 1 : 0;

    std::cout << "CGI response sent: " << totalSent << "/" << responseSize << " bytes" << std::endl;

    if (totalSent == responseSize)
    {
        this->state = ResponseDone;
    }
}

// ?? something missing
// send repsonse and close cfd for GET!!!.
// this response if for GET REQUEST ONLLY IF IN CONFIG FILE A PATH ISNT ALLOWED GET RETURNS 405 Method Not Allowed
// web
int Response::check_is_file(const char *path) {
    struct stat sb;
    
    if (stat(path, &sb) == -1) {
        std::cerr << "file doesn't exist or permission denied" << std::endl;
        return -1;  // stat error (file doesn't exist or permission denied)
    }
    
    if (S_ISREG(sb.st_mode)) {
        // It’s a regular file
        return 0;
    } else if (S_ISDIR(sb.st_mode)) {
        // It’s a directory — return error
        std::cerr << "Error: Path is a directory" << std::endl;
        return -2;
    } else {
        // It’s something else (symlink, socket, fifo, etc.)
        std::cerr << "Error: Path is not a regular file\n" << std::endl;
        return -3;
    }
}

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
        if (this->settings.redirected == true)
            return (redirectResponse(cfd, this->filePath.c_str()));
        if (this->settings.autoIndexed && this->settings.dirUrl)
            return (DirectoryListing(cfd, this->filePath, errorPages));
        if (!this->settings.GET)
        {

            return errorResponsePage(cfd, errorPages, 405);
            // return (notAllowedGetResponse(cfd));
        }
        // checking if the path actualy exist and allowed to be accessed.
        if (access(this->filePath.c_str(), F_OK) == 0)
        {
            if ((check_is_file(this->filePath.c_str()) != 0) || (access(this->filePath.c_str(), R_OK) == -1))
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

    // new : for cgi merge : handles cgi and static pages
    if (isCgiScript(this->filePath))
    {
        std::cout << "Processing CGI request for: " << this->filePath << std::endl;
        return (handleCgiRequest(this->filePath, cfd, req, errorPages));
    }
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
    else if (req->_requestErrors.ContentTooLarge)
    {
        ofs << "HTTP/1.1 413 Request Entity Too Large \r\n"
            << "\r\n";
    }
    std::string resp = ofs.str();
    if (send(cfd, resp.c_str(), resp.length(), MSG_NOSIGNAL) == -1)
    {
        std::cout << "send error in handle bad Request" << std::endl;
        throw (cfd);
    }
    this->state = ResponseDone;
}

// header of a successful post response.
void Response::successPostResponse(int cfd)
{
    std::ostringstream ofs;
    ofs << "HTTP/1.1 201 Created \r\n";
    addCookiesHeader(ofs);
    ofs << "Connection: Close \r\n"
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

void Response::sendTimedOutResponse(int cfd, Request* req)
{
    std::ostringstream ofs;
    ofs << "HTTP/1.1 408 Request Timeout\r\n";
    if (req->getState() != 0)
        addCookiesHeader(ofs);
    ofs << "Content-Length: 0\r\n"
        << "\r\n";
    std::string msg = ofs.str();
    if (send(cfd, msg.c_str(), msg.length() , MSG_NOSIGNAL) == -1)
        throw (cfd);
    this->state = ResponseDone;
}



// code for cgi response

void Response::setCgiBody(const std::string& body)
{
    this->cgi_body = body;
    this->is_cgi_response = true;
    this->res_data.clength = body.length();
}

std::string Response::getStatusMessage(int code)
{
    if (statuscodes == NULL) {
        switch(code) {
            case 200: return "OK";
            case 201: return "Created";
            case 204: return "No Content";
            case 400: return "Bad Request";
            case 401: return "Unauthorized";
            case 403: return "Forbidden";
            case 404: return "Not Found";
            case 405: return "Method Not Allowed";
            case 500: return "Internal Server Error";
            case 501: return "Not Implemented";
            case 502: return "Bad Gatewa    y";
            case 503: return "Service Unavailable";
            case 504: return "Gateway Timeout";
            default: return "Unknown Status";
        }
    }    
    std::ostringstream status_key;
    status_key << code;    
    std::map<std::string, std::string>::iterator it = statuscodes->find(status_key.str());
    if (it != statuscodes->end()) {
        return it->second;
    }
    return "Unknown Status";
}

void Response::addHeader(const std::string& key, const std::string& value)
{
    this->cgi_headers[key] = value;
}

void Response::setStatusCode(int code)
{
    this->status_code = code;
}

void Response::setContentType(const std::string& contentType)
{
    this->res_data.contentType = contentType;
    this->cgi_headers["Content-Type"] = contentType;
}

/**
* Get the CGI body content
* @return CGI response body
*/
const std::string& Response::getCgiBody() const
{
    return this->cgi_body;
}

/**
* Get CGI headers
* @return Map of CGI headers
*/

const std::map<std::string, std::string>& Response::getCgiHeaders() const
{
    return this->cgi_headers;
}
/**
* Get HTTP status code
* @return HTTP status code
*/
int Response::getStatusCode() const
{
    return this->status_code;
}

bool Response::isCgiResponse() const
{
    return this->is_cgi_response;
}

void Response::setBody(const std::string& body)
{
    if (this->is_cgi_response) {
        setCgiBody(body);
    } else {
        this->response.str("");
        this->response << body;
        this->res_data.clength = body.length();
        }
}

std::string Response::buildCgiResponse()
{
    if (!is_cgi_response) {
            return "";
        }
    std::ostringstream response_stream;
    response_stream << "HTTP/1.1 " << status_code << " ";    
    std::ostringstream status_key;
    status_key << status_code;    
    if (statuscodes != NULL) {
        std::map<std::string, std::string>::iterator it = statuscodes->find(status_key.str());
    if (it != statuscodes->end()) {
            response_stream << it->second;
    } else {
            response_stream << "Unknown Status";
        }
    } else {
        switch(status_code) {
            case 200: response_stream << "OK"; break;
            case 404: response_stream << "Not Found"; break;
            case 500: response_stream << "Internal Server Error"; break;
            case 504: response_stream << "Gateway Timeout"; break;
            default: response_stream << "Unknown Status"; break;
        }
    }
    response_stream << "\r\n";  
    for (std::map<std::string, std::string>::const_iterator header_it = cgi_headers.begin();
        header_it != cgi_headers.end(); ++header_it) {
        response_stream << header_it->first << ": " << header_it->second << "\r\n";
    }    
    if (cgi_headers.find("Content-Length") == cgi_headers.end()) {
        response_stream << "Content-Length: " << cgi_body.length() << "\r\n";
    }    
    if (cgi_headers.find("Content-Type") == cgi_headers.end()) {
        response_stream << "Content-Type: text/html\r\n";
    }    
    if (_request && _request->isAlive()) {
        response_stream << "Connection: keep-alive\r\n";
    } else {
        response_stream << "Connection: close\r\n";
    }
    addCookiesHeader(response_stream);
    response_stream << "\r\n"; 
    response_stream << cgi_body;
    return response_stream.str();
}

void Response::resetCgiData()
{
    this->cgi_body.clear();
    this->cgi_headers.clear();
    this->status_code = 200;
    this->is_cgi_response = false;
    this->res_data.clength = 0;
}

bool Response::hasCgiHeader(const std::string& headerName) const
{
    return cgi_headers.find(headerName) != cgi_headers.end();
}

std::string Response::getCgiHeader(const std::string& headerName) const
{
    std::map<std::string, std::string>::const_iterator it = cgi_headers.find(headerName);
    if (it != cgi_headers.end()) {
        return it->second;
    }
    return "";
}

void Response::mergeCgiResponse()
{
    if (is_cgi_response) {
        std::string full_response = buildCgiResponse();
        this->response.str("");
        this->response << full_response;
        this->resp_msg = this->response.str().c_str();
        }
}

bool Response::isCgiScript(const std::string &requestPath)
{
    size_t lastDot = requestPath.find_last_of('.');
    if (lastDot == std::string::npos)
    {
        return false;
    }

    std::string extension = requestPath.substr(lastDot);
    return (extension == ".py" ||
            extension == ".pl" ||
            extension == ".php" ||
            extension == ".rb" ||
            extension == ".sh" ||
            extension == ".cgi");
}

std::string formatDate(time_t rawtime)
{
    char buffer[20];
    std::tm* timeinfo = std::localtime(&rawtime);
    std::strftime(buffer, sizeof(buffer), "%d-%b-%Y %H:%M", timeinfo);
    return std::string(buffer);
}

std::string humanSize(off_t size)
{
    std::ostringstream oss;
    if (size >= (1 << 20))
        oss << std::fixed << std::setprecision(1) << (size / (1024.0 * 1024)) << "M";
    else if (size >= (1 << 10))
        oss << (size / 1024) << "K";
    else
        oss << size << "B";
    return oss.str();
}

// <filname.jpg, 12-MAR-2025 10:15 134K>
// path  is ./pages/images
std::map<std::string, std::string> Response::getDirectoryEntries(std::string& path)
{
    std::cout << "directory in which we are listing... " << path << std::endl;
    std::map<std::string, std::string> res;
    std::string fullpath;
    std::cout << "directory to open is " << path << std::endl;
    DIR* dir = opendir(path.c_str());
    struct stat st;
    if (dir == 0)
    {
        std::cout << "directory " << path.c_str() << " not found" << std::endl;
        throw (1);
        //throw ("opendir");
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != 0)
    {
        fullpath = path + "/" + entry->d_name;
        std::cout << "fullpath is " << fullpath << std::endl;
        if (stat(fullpath.c_str(), &st) == 0 && S_ISREG(st.st_mode))
        {
            std::ostringstream line;
            line << std::setw(20) << formatDate(st.st_mtime)
                << std::right << std::setw(6) << humanSize(st.st_size);
            res.insert(std::make_pair(entry->d_name, line.str()));
        }
    }
    closedir(dir);
    return res;
}
// dir = ./pages/images
void Response::DirectoryListing(int cfd, std::string& dir, std::map<int, std::string>& errorPages)
{

    std::map<std::string, std::string> entries_info;
    try {
        entries_info = getDirectoryEntries(dir);}
    catch (int)
    {
        // non existing direction
        if (!this->settings.GET)
        {

            return errorResponsePage(cfd, errorPages, 405);
            // return (notAllowedGetResponse(cfd));
        }
        // checking if the path actualy exist and allowed to be accessed.
        if (access(this->filePath.c_str(), F_OK) == 0)
        {
            if ((check_is_file(this->filePath.c_str()) != 0) || (access(this->filePath.c_str(), R_OK) == -1))
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
    std::ostringstream body;

    body << "<html>\n"
        << "<head><title>Index of " << dir << "</title></head>\n"
        << "<body>\n"
        << "<h1>Index of " << dir << "</h1><hr><pre>\n"
        << "<a href=\"../\">../</a>\n";
    std::map<std::string, std::string>::iterator it;
    for (it = entries_info.begin(); it != entries_info.end(); ++it)
    {
        body << "<a href=\"" << it->first << "\">" << it->first << "</a>" << std::setw(20) << it->second << "\n";
    }
    body << "</pre><hr>\n"
        << "</body>\n"
        << "</html>\n";
    std::string body_str = body.str();
    this->res_data.contentType = "text/html";
    chunked = false;
    std::string header = RspHeader(body_str.size(),200);
    if (send(cfd, header.c_str(), header.length(), MSG_NOSIGNAL) == -1)
    {
        this->state = ResponseDone;
        throw(cfd);
    }
    if (send(cfd, body_str.c_str(), body_str.length(),MSG_NOSIGNAL) == -1)
    {
        this->state = ResponseDone;
        throw(cfd);
    }
    this->state = ResponseDone;
}