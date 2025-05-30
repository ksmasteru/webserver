#include "../includes/Request.hpp"
#include <map>
#include <string>
#include <iostream>
#include <cstring>
#include "../includes/webserver.hpp"

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
        && str[3] == 'E' && str[4] == 'T' && str[5] == 'E'
            && str[6] == ' ');
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
    MainState = ReadingRequestHeader;
    SubState = start;
    openPostfd = false;
    openRequestFile = false;
    totLent = 0;
}

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
    if (this->SubState < name)
        parseRequestLine(request, readBytes, offset); // increment offset;
    std::cout << "after parsing request line offset is "<< offset << " susbstate " << SubState << std::endl;
    for (; offset < _bytesread; offset++)
    {
        c = request[offset];
        switch (this->SubState)
        {
            case name:
                if (c == '\r' || c == '\n')
                {
                    this->SubState = LF;
                    break;
                }
                if (isspace(c))
                    throw("bad request field name");
                if (c == ':')
                {
                    if (fieldname.empty())
                        throw ("empty field name\n"); 
                    this->SubState = OWS1;
                }
                else
                    fieldname += c;
                break;
            case OWS1:
                this->SubState = val;
                if (isspace(c))
                    break;
            case val:
                if (c == '\r')
                {
                    if (fieldvalue.empty())
                        throw("emptyvalue\n");
                    this->SubState = CR;
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
                    this->SubState = LF;
                    break;
                }
                else if (c == '\n')
                    this->SubState = LF;
                else
                {
                    throw ("bad CR case\n");
                }
            case LF:/*code changed here*/
                if (c != '\n')
                    throw ("no new line\n");
                if(fieldname.empty()) /*it skiped from name to here.*/
                {
                    // in this case we move to body parsing.
                    this->MainState = ReadingRequestBody;
                    offset++;
                    return (parseRequestBody(request,offset,readBytes));
                }
                headers[fieldname] = fieldvalue;
                fieldvalue.clear();
                fieldname.clear();
                this->SubState = name;
                break;
            default:
                std::cout << "bad value for request line" << std::endl;
                return ;
                break;
        }
    }
    if (this->SubState != name || !fieldname.empty())
        throw ("bad request field\n");
    else
    {
        this->SubState = doneParsing;
        if (this->getType().compare("GET") == 0 || this->getType().compare("DELETE") == 0)
        {
            if (offset != _bytesread)
                std::cout << "body of request has been ignored" << std::endl; 
            this->MainState = Done;
        }
        else
            this->MainState = ReadingRequestBody;
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
    this->SubState = start;
    char c;
    int first;
    for (;offset < readBytes; offset++)
    {
        c = request[offset];
        switch (this->SubState)
        {
            case start:
                first = offset;
                if (c == '\r' || c == '\n')
                    break;
                if ((c < 'A' || c > 'Z') && c != '_' && c != '-')
                    throw ("bad request start");
                // else move on.
                this->SubState = method_name;
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
                    this->SubState = after_method_space;
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
                        this->SubState = request_uri;
                        // also mark the start and end of each field
                        break;
                    }
                    throw("bad request no slash\n");
                case request_uri:
                    switch (c)
                    {
                        case '?':
                            this->SubState = question_mark;
                            break;
                        case ' ':
                            this->SubState = spaceafterurl;
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
                            this->SubState = query_equal_mark;
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
                            this->SubState = spaceafterurl;
                        else
                            this->SubState = querykey;
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
                        this->SubState = queryValue;
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
                            this->SubState = querykey;
                        else
                            this->SubState = queryValue;
                        break;
                    }
                    this->qvalue += c;
                    break;
                case spaceafterurl:
                    if (c != 'H')
                        throw("bad request\n");
                    first = offset;
                    this->SubState = HTTPword;
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
                                    this->SubState = httpgreat;
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
                    this->SubState = dot;
                    break;
                case dot:
                    if (c != '.')
                        throw("no dot in http format\n");
                    this->SubState = httpminor;
                    break;
                case httpminor:
                    if (c != '1' && c != '0')
                        throw("bad http minor format\n");
                    this->httpMinor = c;
                    this->SubState = CR;
                    break;
                case CR:
                    if (c != '\r')
                        throw("bad request\n");
                    this->SubState = LF;
                    break;
                case LF:
                    if (c != '\n')
                        throw ("no new line at end\n");
                    offset++;
                    this->SubState = name;
                    return ;// ?
                default:
                    throw ("wrong state");
                    break;
            }
    }
}
void Request::setUpPostFile()
{
    std::string fileName = generateUniqueFilename();
    //std::string extension = getExtension(); 
    fileName += getExtension();
    std::cout << "filename for upload ist " << fileName << std::endl;
    try {
        this->RequestFile.fd = open(fileName.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
        if (this->RequestFile.fd == -1)/*should throw an error : stop request.*/
            throw ("couldnt open post fd");
    }
    catch (const char* msg)
    {
        std::cout << msg << std::endl;
        exit(1);
    }
    this->RequestFile.offset = 0;
    // setting up transfer type
    if (headers.find("Content-Length") != headers.end())
        this->RequestFile.type = Content_Length;
    else if ((headers.find("Transfer-Encoding") != headers.end())
        && (headers["Transfer-Encoding"] == "chunked"))
        this->RequestFile.type = Chunked;
    else
        throw ("Bad Request");/*RFC 7230*/
    // determine file size based on transfer.
    if (this->RequestFile.type == Content_Length)
        this->RequestFile.size = stringToLong(headers["Content-Length"]);
    else
    {
        this->RequestFile.size = 0;
        this->RequestFile.toWrite = 0;
        this->RequestFile.state = chunk_size;
    }
    this->openRequestFile = true;
}
/* this handles the post request Body : uploads the file to server.*/
void Request::parseRequestBody(char *request, int offset, int readBytes)
{
    std::cout << "parseRequestBody called" << std::endl;
    // if request type is not post ignore
    if (this->getType().compare("POST") != 0)
    {
        this->MainState = Done;
        this->SubState = doneParsing;
        return ;
    }
    // BELW CODE ONLY HANDLES POST.
    if (!openRequestFile)
        setUpPostFile();
    switch(this->RequestFile.type)
    {
        case Content_Length:
            contentLengthBody(request, offset, readBytes);
            break;
        case Chunked:
            chunkedBody(request, offset, readBytes);
            break;
        default:
            break;
    }
}

void Request::chunkedBody(char *request, int offset, int readBytes)
{
    std::cout << "TODO chunkedBody" << std::endl;
    if (!openRequestFile)
        setUpPostFile();
    //request[readBytes] = '\0';
    //std::cout << "Request body ==========================================" << request + offset << std::endl;
    //exit (1);
    size_t start = 0;
    char c;
    size_t writtendata = 0;
    for (;offset < readBytes; offset++)
    {
        c = request[offset];
        start = offset; 
        switch (this->RequestFile.state)
        {
            case chunk_size:
                if(c == '\r')
                {
                    this->RequestFile.state = CR1;
                    break;
                }
                else if ((c >= '0' && c <= '9') || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') || (c <= 'Z')))
                {
                    this->RequestFile.chunk_lent += c;
                    break;
                }
                else
                    throw ("bad chunked encoding");
            case CR1:
                if (c != '\n')
                    throw ("bad chunked encoding missgin LF1");
                if (this->RequestFile.chunk_lent.empty())
                    throw ("bad chunked encoding empty size");
                if ((this->RequestFile.toWrite = hexStringToLong(this->RequestFile.chunk_lent)) == 0)
                {
                    std::cout << "chuck size is " << this->RequestFile.toWrite << std::endl;
                    this->RequestFile.state = CR3;
                    break;
                }
                this->RequestFile.state = LF1;
                break;
            case LF1:
                start = offset;
                this->RequestFile.state = chunk_data;
            case chunk_data:
                if (this->RequestFile.toWrite != 0)
                {
                    writtendata = std::min(readBytes - start, this->RequestFile.toWrite);
                    this->RequestFile.toWrite -= write(this->RequestFile.fd, request + start, writtendata);
                    offset += writtendata - 1;
                    break;
                }
                if (c != '\r')
                    throw ("bad chunked encoding missing CR");
                this->RequestFile.state = CR2;
                break;
            case CR2:
                if (c != '\n')
                    throw ("bad chunked encoding missign CR2");
                start = 0;
                this->RequestFile.chunk_lent.clear();
                this->RequestFile.state = chunk_size;
                break;
            case CR3:
                if (c != '\r')
                    throw ("bad chunked econding missing CR3");
                this->RequestFile.state = LF3;
                break;
            case LF3:
                if (c != '\n')
                    throw ("bad chunked encoding missing LF3");
                this->RequestFile.state = chunk_done;
                this->MainState = Done;
                this->SubState = doneParsing;
                close(this->RequestFile.fd);
                break;
            case chunk_done:
                std::cout << "chunk transfer is done ignoring the remaining data" << std::endl;
                this->RequestFile.state = chunk_done;
                this->MainState = Done;
                this->SubState = doneParsing;
                close(this->RequestFile.fd);
                break;
        }
    }
}

void Request::contentLengthBody(char *request, int offset, int readBytes)
{
    int writtenData = write(this->RequestFile.fd, request + offset,readBytes - offset);
    if (writtenData != readBytes - offset)
        throw ("partial/failed write : internal server error");
    this->RequestFile.offset += writtenData;
    // checck if done.
    if (this->RequestFile.offset == this->RequestFile.size)
    {
        std::cout << "File received completly" << std::endl;
        close(this->RequestFile.fd);
        this->MainState = Done;
    }
}

/*generate a unique fd for post request*/
// Content-Type: text/html; charset=utf-8
std::string Request::getExtension()
{
    std::string extension = ".bin";
    if (headers.find("Content-Type") != headers.end())
        extension = headers["Content-Type"];
    else
        return extension;
    std::map<std::string, std::string> contentMap = {
    {"text/html", ".html"},
    {"image/png", ".ico"},
    {"text/css", ".css"},
    {"application/javascript", ".js"},
    {"application/json", ".json"},
    {"image/jpg", ".jpg"},
    {"image/jpeg", ".jpeg"},
    {"image/gif", ".gif"},
    {"image/svg+xml", ".svg"},
    {"text/plain", ".txt"},
    {"video/mp4", ".mp4"},
    {"", ".bin"}
    };
    return contentMap[extension];
}

int Request::getPostFd()
{
    // generate a unique name.
    std::string fileName = generateUniqueFilename();
    //std::string extension = getExtension(); 
    fileName += getExtension();
    std::cout << "filename for upload ist " << fileName << std::endl;
    try {
    this->Postfd = open(fileName.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (this->Postfd == -1)
        throw ("couldnt open post fd");
    }
    catch (const char* msg)
    {
        std::cout << msg << std::endl;
        exit(1);
    }
    this->openPostfd = true;
    return this->Postfd;
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
    if (headers.find("Connection") != headers.end()
        && headers["Connection"] == "keep-alive") /*case sensitive*/
    {
        std::cout << "Client is Staying alive...!!!" << std::endl;
        return (true);
    }
    return (false);
}

std::string Request::getHttpVersion(){
    std::string version = "HTTP/";
    return (version + this->httpGreater + '.' + this->httpMinor);
}
