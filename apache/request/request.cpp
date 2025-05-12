#include "../includes/Request.hpp"
#include <map>
#include <string>
#include <iostream>
#include <cstring>

bool isGetRequest(char *str)
{
    return (str[0] == 'G' && str[1] == 'E' && str[2] == 'T'
        && str[3] == ' ');
}
bool isPostRequest(char *str)
{
    return (str[0] == 'P' && str[1] == 'O' && str[2] == 'S'
        && str[3] == 'T' && str[4] == ' ');
}

bool isDeleteRequest(char *str)
{
    return (str[0] == 'D' && str[1] == 'E' && str[2] == 'L'
        && str[3] == 'E' && str[4] == 'T' && str[5] == ' ');
}

bool isHttp(char *str)
{
    return (str[0] == 'H' && str[1] == 'T' && str[2] == 'T'
        && str[3] == 'P' && str[4] == '/');
}
void Request::setConnectionType()
{
    // iteate the map and find Connection :
    std::map<std::string, std::string>::iterator it;
    std::string connection = "Connection";
    this->keep_alive = false;
    if ((it = headers.find(connection)) != headers.end())
    {
        if (it->second.compare("keep-alive") == 0)
        {
            std::cout << "found keep-alive" << std::endl;
            this->keep_alive = true;
        }
    }
}
/*if request type != get and there is still data after parssing 
header throw bad request.*/
Request::Request()
{
    mainState = ReadingRequestHeader;
    subState = start;
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

void Request::printRequestLine()
{
    std::cout << "method type " << this->getType() << std::endl; 
    std::cout << "Request Uri " << this->targetUri << std::endl;     
}

void Request::printHeaderFields()
{
    for (auto it = headers.begin(); it != headers.end(); ++it)
        std::cout << it->first << ": " << it->second << std::endl;
}
bool unvalidheaderVal(std::string& val)
{
    size_t i = val.length() - 1;
    int n = 0;
    while (isspace(val[i--]))
    {
        if (n++ > 1)
            return (true);
    }
    return (false);
}

void Request::parseRequestHeader(char* request, int readBytes)
{
    // each time enters with a new char buffer
    char c;
    int offset = 0;
    std::string fieldname, fieldvalue;
    _bytesread = readBytes;
    if (this->subState < name)
        parseRequestLine(request, readBytes, offset); // increment offset;
    std::cout << "after parsing request line offset is "<< offset << " susbstate " << subState << std::endl;
    for (; offset < _bytesread; offset++)
    {
        c = request[offset];
        switch (this->subState)
        {
            case name:
                if (c == '\r' || c == '\n')
                    break;
                if (isspace(c))
                    throw("bad request field name");
                if (c == ':')
                {
                    if (fieldname.empty())
                        throw ("empty field name\n"); 
                    this->subState = OWS1;
                }
                else
                    fieldname += c;
                break;
            case OWS1:
                this->subState = val;
                if (isspace(c))
                    break;
            case val:
                if (c == '\r')
                {
                    if (fieldvalue.empty())
                        throw("emptyvalue\n");
                    this->subState = CR;
                }
                else
                {
                    fieldvalue += c;
                    break;
                }
            case CR:
                if (unvalidheaderVal(fieldvalue))
                    throw ("bad header val\n");
                if (c == '\r')
                {
                    this->subState = LF;
                    break;
                }
                else if (c == '\n')
                    this->subState = LF;
                else
                {
                    throw ("bad CR case\n");
                }
            case LF:
                if (c != '\n')
                    throw ("no new line\n");
                this->subState = name;
                headers[fieldname] = fieldvalue;
                fieldvalue.clear();
                fieldname.clear();
                break;
            default:
                std::cout << "bad value for request line" << std::endl;
                return ;
                break;
        }
    }
    if (this->subState != name || !fieldname.empty())
        throw ("bad request field\n");
    else
    {
        this->subState = doneParsing;
        if (this->getType().compare("GET") == 0 || this->getType().compare("DELETE") == 0)
        {
            if (offset != _bytesread)
            {
                std::cout << "offset: " << offset << " bytes read " << _bytesread << std::endl;
                throw ("bad request body");
            }
            this->mainState = Done;
        }
        else
            this->mainState = ReadingRequestBody;
            // !!!should parse request body with the remaing unread bytes!! readybtes - offset
    }
    std::cout << "request header parsed successfully" << std::endl;
}

void Request::addtoheaders(std::string& key, std::string& value)
{
    headers[key] = value;
}

void Request::parseRequestLine(char *request, int readBytes, int &offset)
{
    this->subState = start;
    char c;
    int first;
    for (;offset < readBytes; offset++)
    {
        c = request[offset];
        switch (this->subState)
        {
            case start:
                first = offset;
                if (c == '\r' || c == '\n')
                    break;
                if ((c < 'A' || c > 'Z') && c != '_' && c != '-')
                    throw ("bad request start");
                // else move on.
                this->subState = method_name;
                break;
            case method_name:
                if (c == ' ')
                {
                    int n = offset - first;
                    switch (n)
                    {
                        case 3: // Get
                            if (isGetRequest(request + first))
                                this->type = "GET";
                            break;
                        case 4: // Post 
                            if (isPostRequest(request + first))
                                this->type = "POST";
                            break;
                        case 6: //  delete
                            if (isDeleteRequest(request + first))
                                this->type = "DELETE";
                            break;
                        default:
                            throw("Bad Request method name\n");
                    } 
                    if (this->type.empty())
                        throw("bad request name\n"); 
                    this->subState = after_method_space;
                    break;
                }
                if ((c < 'A' || c > 'Z') && c != '_' && c != '-')
                    throw("http parse invalid method");
                break;
                case after_method_space: // this code focuses on origin form 
                    //printf("after method space\n");
                    if (c == '/') // makes you jump host parsing
                    {
                        this->targetUri += c;
                        this->subState = request_uri;
                        // also mark the start and end of each field
                        break;
                    }
                    throw("bad request no slash\n");
                case request_uri:
                    switch (c)
                    {
                        case '?':
                            this->subState = question_mark;
                            break;
                        case ' ':
                            this->subState = spaceafterurl;
                            break;
                        default :
                            this->targetUri += c;
                            break;
                    }
                    break;
                case question_mark: // ?'fdsfds'=fdfsd
                    switch (c)
                    {
                        case '=':
                            this->subState = query_equal_mark;
                            break;
                        case ' ':
                            throw("bad request question mark no value\n");
                        case '?':
                            throw("bad request multiple '?'\n");
                        case '&':
                            throw("bad request no key before &\n");
                        default:
                            this->qkey += c;
                            break;
                    }
                    break;
                case query_equal_mark:
                    if (this->qkey.empty())
                        throw("bad request no query key\n");
                    if (c < 0x20 || c == 0x7f || c == '?') /*no printib chars*/
                        throw("bad request uri\n");
                    if(c == ' ' || c == '&')
                    {
                        if (this->qvalue.empty())
                            throw("bad reqeust query done\n");
                        this->queries[this->qkey] = this->qvalue;
                        this->qkey.clear();
                        this->qvalue.clear();
                        if (c == ' ')
                            this->subState = spaceafterurl;
                        else
                            this->subState = querykey;
                        break;
                    }
                    else
                        this->qvalue += c;
                    break;
                case querykey:
                    if (c == '&' || c == ' ' || c == '?')
                        throw("bad request\n");
                    if (c == '=')
                    {
                        this->subState = queryValue;
                        break;
                    }
                    this->qkey += c;
                    break;
                case queryValue:
                    if (this->qkey.empty())
                        throw("bad request : no query key\n");
                    if (c == '?')
                        throw("bad request multiple '?'\n");
                    if (c == '&' || c == ' ')
                    {
                        if (this->qvalue.empty())
                            throw("bad reqeust query done\n");
                        this->queries[this->qkey] = this->qvalue;
                        this->qkey.clear();
                        this->qvalue.clear();
                        if (c == '&')
                            this->subState = querykey;
                        else
                            this->subState = queryValue;
                        break;
                    }
                    this->qvalue += c;
                    break;
                case spaceafterurl:
                    if (c != 'H')
                        throw("bad request\n");
                    first = offset;
                    this->subState = HTTPword;
                    break;
                case HTTPword:
                    switch (c)
                    {
                        case '/':
                        {
                            int n = offset - first;
                            switch(n)
                            {
                                case 4:
                                    if (!isHttp(request + first))
                                        throw("bad http word1\n");
                                    this->subState = httpgreat;
                                    break;
                                default:
                                    throw("bad http word2\n");
                            }
                        }
                        default:
                            break;
                    }
                break;
                case httpgreat:
                    if (c != '1' && c != '0')
                        throw("bad http version\n");
                    this->httpGreater = c;                
                    this->subState = dot;
                    break;
                case dot:
                    if (c != '.')
                        throw("no dot in http format\n");
                    this->subState = httpminor;
                    break;
                case httpminor:
                    if (c != '1' && c != '0')
                        throw("bad http minor format\n");
                    this->httpMinor = c;
                    this->subState = CR;
                    break;
                case CR:
                    if (c != '\r')
                        throw("bad request\n");
                    this->subState = LF;
                    break;
                case LF:
                    if (c != '\n')
                        throw ("no new line at end\n");
                    offset++;
                    this->subState = name;
                    return ;// ?
                default:
                    throw ("wrong state");
                    break;
            }
    }
}

void Request::parseRequestBody(char *request, int readBytes)
{
    // TO DO when handling post request.
    std::cout << "say hi" << std::endl;
}

// to avoid copying the map each time. 
std::string Request::getMapAtIndex(unsigned int index)
{
    return (parse_map[index]);
}

std::string Request::getRequestPath()
{
    return (this->targetUri);
}

bool Request::isAlive()
{
    return (this->keep_alive);
}
std::string Request::getHttpVersion(){
    std::string version = "HTTP/";
    return (version + this->httpGreater + '.' + this->httpMinor);
}
