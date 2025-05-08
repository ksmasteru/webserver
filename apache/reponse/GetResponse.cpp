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
        {"mp4", "text/html"},
        {"", "text/plain"}
    };
    return contentMap[extension];
}

std::string GetResponse::handleBinaryFile(const char* path, std::string& extension)
{
    std::cout << "reading a binary file" << std::endl;
    std::ifstream ifs;
    ifs.open(path, std::ios::binary);
    if (!ifs)
        throw ("couldnt open binary file\n");
    // determine file size
    ifs.seekg(0, std::ios::end);
    std::streamsize size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    
    std::string buffer(size, '\0');
    if(ifs.read(&buffer[0], size))
        std::cout << "Read " << size << " bytes into string.\n";
    else
        std::cout << "failed to read file" << std::endl;
    this->res_data.clength = size;
    this->res_data.contentType = content(extension);
    return (buffer);
}

std::string GetResponse::requestPageBody(const char* path)
{
    // TODO load actual request page
    // '/pages should be appended to path
    std::ifstream ifs;
    this->res_data.status = 200; // TODO
    this->res_data.keepAlive = (_request->isAlive()) ? "Keep-Alive" : "Close";
    size_t extension_pos = this->_request->getRequestPath().rfind(".");
    std::string extension = (extension_pos != std::string::npos) ? this->_request->getRequestPath().substr(extension_pos+1) : "";
    if (strncmp(path,"pages/", sizeof("pages/")) == 0)
    {
        std::cout << "index path" << std::endl;
        extension = "html";
        ifs.open("pages/index.html");
    }
    else if (access(path, F_OK) == -1)
    {
        this->res_data.status = 404;
        extension = "html";
        std::cout << "couldnt find " << path << std::endl;
        ifs.open("pages/404.html");
    }
    else if (access(path, R_OK) == -1)
    {
        this->res_data.status = 403;
        extension = "html";
        ifs.open("pages/403.html");
    }
    else if (extension.compare("ico") == 0 || extension.compare("png") == 0
        || extension.compare("gif") == 0)
            return (handleBinaryFile(path, extension));
    else
    {
        ifs.open(path);
        extension_pos = this->_request->getRequestPath().rfind(".");
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
    std::cout << "for path " << path << "content type is " << this->res_data.contentType << std::endl;
    return responseBody;
}

void GetResponse::makeResponse()
{
    std::string path = "pages" + this->_request->getRequestPath();
    std::string pageBody = requestPageBody(path.c_str());
    response << RspHeader(this->res_data.clength, this->res_data.status)
            << "\r\n"
            << pageBody;
    char *reponse = new char[this->res_data.totallength + 1];
    strncpy(reponse, response.str().c_str(), this->res_data.totallength);
    reponse[this->res_data.totallength] = '\0';
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

size_t  GetResponse::getSize()
{
    return (this->res_data.totallength);
}

