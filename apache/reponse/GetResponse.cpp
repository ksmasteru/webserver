#include "../includes/GetResponse.hpp"

GetResponse::GetResponse(const std::string& type, Request* req) : AResponse(type, req)
{
    // first checks if the file exist based on this info : fill body header
    // shalow copy of request.
    std::cout << "Getresponse constructor called " << std::endl;
    std::cout << "type is " << type << " request ptr ist " << req << std::endl;
    const char* path = _request->getRequestPath();
    std::cout << "path is " << path << std::endl;
}

// response header should be last to get filled.

std::string RspStatusline(unsigned int code)
{
    // HTTP/{VERSION(DOUBLE)} {STATUS CODE} RESPONSE
    // HTTP/1.1 200 OK
    // map response to code.
    std::string http_version = "HTTP/1.1";
    std::string statusCode = intToString(code);
    std::string Response = "OK"; // reponse message.
    std::string rsp = http_version + " " + statusCode + " " + Response + "\r\n";
    return (rsp);
}

std::string getTime()
{
    std::time_t now = std::time(nullptr);
    std::tm *gmt = std::gmtime(&now);  // Get GMT time

    char buffer[100];
    // Format: "Mon, 22 Apr 2025 10:00:00 GMT"
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmt);
    return (buffer);
    
}

std::string RspHeader(unsigned int cLength, const char *keepAlive)
{
    // make this work with a struct  :
    // date:
    // server:
    // content-type:
    // content-length:
    std::string linebuf = "\r\n";
    std::string getCurrentTime = "date: " + getTime() + linebuf;
    std::string server =  "server: apache/2.4.41 (Ubuntu)" + linebuf;
    std::string contentType = "content-Type: text/html; charset=UTF-8" + linebuf;
    std::string contentLength =  "content-Length: " + intToString(cLength)+ linebuf;
    std::string alive = keepAlive + linebuf; // TODO
    std::string res = getCurrentTime + server + contentType + contentLength + "Connection: " + alive;
    return res;
}

std::string GetResponse::generateHeader()
{
  /*  std::string header = "HTTP/1.1 402 File not found \r\n"
                                   "Content-Type: text/html\r\n"
                                   "Connection: close\r\n"
                                   "\r\n"; */
    std::string header = RspStatusline(this->res_data.status) + RspHeader(this->res_data.clength, this->res_data.keepAlive);
    return (header);
}



std::string GetResponse::pageBodyError(unsigned int code)
{
    std::cout << "page Body error called" << std::endl;
    std::ifstream file;
    if (code == 403)
        file.open("403.html");
    else if (code == 403)
        file.open("403.html"); // TODO load error page
    else
        file.open("403.html");
    if (!file)
        throw ("opa file 76:getrepsonse");
    std::string resp_buff;
    std::stringstream response_buffer(resp_buff);
    response_buffer << file.rdbuf();
    std::string bodyError = response_buffer.str();
    this->res_data.clength = bodyError.length();
    this->res_data.extension = "text/html"; // TODO
    this->res_data.keepAlive =  "close";
    this->res_data.status = code;
    return (bodyError);
}

std::string GetResponse::requestPageBody(const char* path)
{
    // TODO load actual request page
    std::ifstream ifs;
    ifs.open("/home/hes-saqu/Desktop/webserver/apache/reponse/403.html");
    if (!ifs)
        throw ("requestPageBody couldnt open request file");
    std::string resp_buff;
    std::stringstream  response_buffer(resp_buff);
    response_buffer << ifs.rdbuf();
    std::string responseBody = response_buffer.str();
    this->res_data.clength = responseBody.length(); 
    this->res_data.extension = "text/html"; // TODO
    this->res_data.keepAlive =  "close"; //TODO
    this->res_data.status = 200; // TODO
   // if (map.find(6) != map.end() && map[6].find("keep-alive") != std::string::npos)
        //this->res_data.keepAlive = "keep-alive";
    return responseBody;
}

std::string GetResponse::generateBody(const char* path)
{
    unsigned int responseCode = 200;
    std::ifstream file;
    /*std::cout << "path is " << path << std::endl;
    if (access(path, F_OK) == -1)
        return (pageBodyError(403));
    else if (access(path, R_OK) == -1)
        return (pageBodyError(404));
    else*/
        return (requestPageBody(path));
}


void GetResponse::makeResponse()
{
    // type of response is based on the file asked.
    // cannot use [0] because request is marked at const and [] can increase
    // the size of the map container.\ 
    std::cout << "makeResponse called" << std::endl;
    std::string path = "./static_files" + _request->getMapAtIndex(0);
    std::string responseBody = generateBody(path.c_str());
    // resp_h is already filled by generate body .
    std::string responseHeader = generateHeader();
    std::string response = (responseHeader + "\n" + responseBody).c_str();
    //std::cout << "makeResponse done" << response <<std::endl;
    this->resp_msg = response.c_str(); // BIG MISTAKE
    size_t len = response.length();
    std::cout << "len is" << len << std::endl;
    char *resp = new char[len];
    resp[len - 1] = '\0';
    std::strncpy(resp, this->resp_msg, len - 1); // you get away with this. 
    this->resp_msg = resp;
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

// has to be used on pla