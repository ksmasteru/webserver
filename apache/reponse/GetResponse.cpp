#include "../includes/GetResponse.hpp"

GetResponse::GetResponse(const std::string& type, Request* req, std::map<std::string, 
std::string>*status) : AResponse(type, req, status)
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
    std::string alive = "Close"; // will be set later;
    std::ostringstream header;
    header  << RspStatusline(code)
            << "Date: " + getTime() + " \r\n"
            << "Server: apache/2.4.41 (Ubuntu) \r\n"
            << "Content-Type: " + this->res_data.contentType + " \r\n"
            << "Content-Length: " + intToString(cLength) + " \r\n"
            << "Connection " + alive + " \r\n";
    return (header.str());
}

std::string content(std::string extension )
{
   std::map<std::string, std::string> contentMap = {
        {"html", "text/html"},
        {"htm",  "text/html"},
        {"css",  "text/css"},
        {"js",   "application/javascript"},
        {"json", "application/json"},
        {"png",  "image/png"},
        {"jpg",  "image/jpg"},
        {"jpeg", "image/jpeg"},
        {"gif",  "image/gif"},
        {"svg",  "image/svg+xml"},
        {"txt",  "text/plain"},
        {"mp4", "text/html"},
        {"", "text/plain"}
    };
    return contentMap[extension];
}

std::string GetResponse::requestPageBody(const char* path)
{
    // TODO load actual request page
    // '/pages should be appended to path
    std::ifstream ifs;
    this->res_data.status = 200; // TODO
    // index path
    std::string extension = "html"; // default for index page and error pages.
    if (strncmp(path,"pages/", sizeof("pages/")) == 0)
    {
        std::cout << "index path" << std::endl;
        ifs.open("pages/index.html");
    }
    else if (access(path, F_OK) == -1)
    {
        this->res_data.status = 404;
        ifs.open("pages/404.html");
    }
    else if (access(path, R_OK) == -1)
    {
        this->res_data.status = 403;
        ifs.open("pages/403.html");
    }
    else
    {
        size_t extension_pos = this->_request->getRequestPath().rfind(".");
        extension = (extension_pos != std::string::npos) ? this->_request->getRequestPath().substr(extension_pos+1) : "";
    }
    if (!ifs)
        throw ("requestPageBody couldnt open request file");
    std::cout << "extension is " << extension << std::endl;
    std::string resp_buff;
    std::stringstream  response_buffer(resp_buff);
    response_buffer << ifs.rdbuf();
    std::string responseBody = response_buffer.str();
    this->res_data.clength = responseBody.length();
    this->res_data.contentType = content(extension);
    this->res_data.keepAlive = (_request->isAlive()) ? "Keep-Alive" : "Close";
    return responseBody;
}

void GetResponse::makeResponse()
{
    std::string path = "pages" + this->_request->getRequestPath();
    response << RspHeader(this->res_data.clength, this->res_data.status)
            << "\r\n"
            << requestPageBody(path.c_str());
    size_t len = response.str().length();
    char *reponse = new char[len + 1];
    strncpy(reponse, response.str().c_str(), len);
    reponse[len] = '\0';
    this->resp_msg = reponse;
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
    std::cout << "KA BOOM" << std::endl;
}
