#include "../includes/Request.hpp"
#include <map>
#include <string>
#include <iostream>

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

Request::Request(char* buffer, int bytesread)
{
    this->RawRequest = buffer;
    _bytesread = bytesread;
    this->state = start;
    offset = 0;
    parseRequest(buffer);
    setConnectionType();
    //this->requestPath = this->getMapAtIndex(0);
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
    // val should at most have one white at the end.
    size_t i = val.length() - 1;
    int n = 0;
    while (isspace(val[i--]))
    {
        if (n++ > 1)
            return (true);
    }
    return (false);
}

void Request::parseRequest(char* request)
{
    //offset and total read
    enum
    {
        name,
        OWS1,
        val,
        CR,
        LF,
    }fieldstate;
    char c;
    fieldstate = name;
    std::string fieldname, fieldvalue;
    if (!parseRequestLine(request))
        throw ("bad request\n");
    for (; offset < _bytesread; offset++)
    {
        c = request[offset];
        switch (fieldstate)
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
                    fieldstate = OWS1;
                }
                else
                    fieldname += c;
                break;
            case OWS1:
                fieldstate = val;
                if (isspace(c))
                    break;
            case val:
                if (c == '\r')
                {
                    if (fieldvalue.empty())
                        throw("emptyvalue\n");
                    fieldstate = CR;
                }
                else
                {
                    fieldvalue += c;
                    break;
                }
            case CR: // this is optional.
                // field value should have more than one space
                // at the end.
                if (unvalidheaderVal(fieldvalue))
                    throw ("bad header val\n");
                if (c == '\r')
                {
                    fieldstate = LF;
                    break;
                }
                else if (c == '\n')
                    fieldstate = LF;
                else
                {
                    throw ("bad CR case\n");
                }
            case LF:
                if (c != '\n')
                    throw ("no new line\n");
                fieldstate = name;
                headers[fieldname] = fieldvalue;
                fieldvalue.clear();
                fieldname.clear();
                break;
        }
    }
    if (fieldstate != name || !fieldname.empty())
        throw ("bad request field\n");
    //printHeaderFields();
}

void Request::addtoheaders(std::string& key, std::string& value)
{
    headers[key] = value;
}
bool Request::parseRequestLine(char *request)
{
    enum{
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
        CR,
        LF
    }state;
    char c;
    state = start;
    int first;
    for (;offset < _bytesread; offset++)
    {
        c = request[offset];
        switch (state)
        {
            case start:
                first = offset;
                if (c == '\r' || c == '\n') // skips this line ?
                    break;
                if ((c < 'A' || c > 'Z') && c != '_' && c != '-')
                    return (printf("bad request start\n"));
                // else move on.
                state = method_name;
                break;
            case method_name: // check valid characters till space
                if (c == ' ')
                {
                    // calculate the diff between begin and end;
                    //printf("i - 1first is %d\n", offset - first);
                    int n = offset - first;
                    //printf("to compare is %s\n", offset + start);
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
                            return (printf("Bad Request method name\n"));
                    } 
                    if (this->type.empty())
                        return (printf("bad request name\n")); 
                    state = after_method_space;
                    break;
                }
                if ((c < 'A' || c > 'Z') && c != '_' && c != '-')
                    return (printf("http parse invalid method"));
                break;
                case after_method_space: // this code focuses on origin form 
                    //printf("after method space\n");
                    if (c == '/') // makes you jump host parsing
                    {
                        this->targetUri += c;
                        state = request_uri;
                        // also mark the start and end of each field
                        break;
                    }
                    return (printf("bad request no slash\n"));
                case request_uri: // /path?key1=val1&key2=val2&key3=val3
                    // request uri form first char to ? or space
                    //printf("request uri c:%c\n",c);
                    switch (c)
                    {
                        case '?':
                            state = question_mark;
                            break;
                        case ' ':
                            state = spaceafterurl;
                            break;
                        default :
                            this->targetUri += c;
                            break;
                    }
                    break;
                case question_mark: // ?'fdsfds'=fdfsd
                    printf("question mark\n");
                    switch (c)
                    {
                        case '=':
                            state = query_equal_mark;
                            break;
                        case ' ':
                            return printf("bad request question mark no value\n");
                        case '?':
                            return printf("bad request multiple '?'\n");
                        case '&':
                            return printf("bad request no key before &\n");
                        default:
                            this->qkey += c;
                            break;
                    }
                    break;
                case query_equal_mark:
                    printf("q equal mark c:%c\n", c);
                    if (this->qkey.empty())
                        return (printf("bad request no query key\n"));
                    if (c < 0x20 || c == 0x7f || c == '?') /*no printib chars*/
                        return (printf("bad request uri\n"));
                    if(c == ' ' || c == '&')
                    {
                        if (this->qvalue.empty())
                            return (printf("bad reqeust query done\n"));
                        this->queries[this->qkey] = this->qvalue;
                        this->qkey.clear();
                        this->qvalue.clear();
                        if (c == ' ')
                            state = spaceafterurl;
                        else
                            state = querykey;
                        break;
                    }
                    else
                        this->qvalue += c;
                    break;
                case querykey:
                    if (c == '&' || c == ' ' || c == '?')
                        return (printf("bad request\n"));
                    if (c == '=')
                    {
                        state = queryValue;
                        break;
                    }
                    this->qkey += c;
                    break;
                case queryValue:
                    if (this->qkey.empty())
                        return printf("bad request : no query key\n");
                    if (c == '?')
                        return (printf("bad request multiple '?'\n"));
                    if (c == '&' || c == ' ')
                    {
                        if (this->qvalue.empty())
                            return (printf("bad reqeust query done\n"));
                        this->queries[this->qkey] = this->qvalue;
                        this->qkey.clear();
                        this->qvalue.clear();
                        if (c == '&')
                            state = querykey;
                        else
                            state = queryValue;
                        break;
                    }
                    this->qvalue += c;
                    break;
                case spaceafterurl:
                    if (c != 'H')
                        return printf(("bad request\n"));
                    first = offset;
                    state = HTTPword;
                    break;
                case HTTPword:
                    switch (c)
                    {
                        case '/':
                        {
                            int n = offset - first;
                            //printf("first is %d n is %d\n", first, n);
                            switch(n)
                            {
                                case 4:
                                    if (!isHttp(request + first))
                                        return printf(("bad http word1\n"));
                                    state = httpgreat;
                                    break;
                                default:
                                    return (printf("bad http word2\n"));
                            }
                        }
                        default:
                            break;
                    }
                break;
                case httpgreat:
                    if (c != '1' && c != '0')
                        return (printf("bad http version\n"));
                    this->httpGreater = c;                
                    state = dot;
                    break;
                case dot:
                    if (c != '.')
                        return (printf("no dot in http format\n"));
                    state = httpminor;
                    break;
                case httpminor:
                    if (c != '1' && c != '0')
                        return printf("bad http minor format\n");
                    this->httpMinor = c;
                    state = CR;
                    break;
                case CR: // demo parsing 
                    if (c != '\r')
                        return printf("bad request\n");
                    state = LF;
                    break;
                case LF:
                    if (c != '\n')
                        throw ("no new line at end\n");
                    offset++;
                    return true;
            }
    }
    return (false);
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
