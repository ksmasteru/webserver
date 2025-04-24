#include "../includes/Request.hpp"
#include <map>
#include <string>
#include <iostream>

Request::Request(char* buffer)
{
    std::cout << "Request constructor" << std::endl;
    this->RawRequest = buffer;
    parseRequest(buffer);
    if (this->RawRequest.find("GET") != std::string::npos) //TODO : better way to define type
        this->type = "GET";
    else
        this->type = "random";
}

/*const std::map<int, std::string>&   Request::getMap() const{
    return (this->parse_map);
}*/

const std::string&   Request::getRawRequest() const{
    return (this->RawRequest);
}

const std::string&   Request::getType() const{
    return (this->type);
}

void Request::parseRequest(char* request)
{
    int start = 5;
    int index = 0;
    size_t end ;
    std::string str;
    std::string reqcopy = request;
    while ((end = reqcopy.find('\n')) != std::string::npos)
    {
        if (!index)
        {
            str = reqcopy.substr(4, end - 14);
            this->parse_map[index] = str;
            index++;
        }
        else
        {
            start = reqcopy.find(' ');
            this->parse_map[index++] = reqcopy.substr(start + 1, (end - start) - 2);
        }
        reqcopy = reqcopy.substr(end + 1);
    }
}
// to avoid copying the map each time. 
std::string Request::getMapAtIndex(unsigned int index)
{
    return (parse_map[index]);
}