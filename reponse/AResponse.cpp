#include "../includes/AResponse.hpp"

AResponse::AResponse(std::string type, Request* req, std::map<std::string, std::string>* status, int client_fd) :_type(type), _request(req), resp_msg(NULL), statuscodes(status)
    , _client_fd(client_fd){
        std::cout << "AResponse constructor called" << std::endl;
        this->statuscodes = new std::map<std::string, std::string>({
                {"200", "OK"},
                {"404", "Not Found"},
                {"500", "Internal Server Error"},
    });
}

AResponse::AResponse(): status_code(200), is_cgi_response(false){
    this->statuscodes = new std::map<std::string, std::string>({
        {"200", "OK"},
        {"201", "Created"},
        {"204", "No Content"},
        {"301", "Moved Permanently"},
        {"302", "Found"},
        {"304", "Not Modified"},
        {"400", "Bad Request"},
            {"401", "Unauthorized"},
            {"403", "Forbidden"},
            {"404", "Not Found"},
            {"405", "Method Not Allowed"},
            {"413", "Payload Too Large"},
            {"414", "URI Too Long"},
            {"500", "Internal Server Error"},
            {"501", "Not Implemented"},
            {"502", "Bad Gateway"},
            {"503", "Service Unavailable"},
            {"504", "Gateway Timeout"}
        });
}

struct resp_h AResponse::getResData()
{
    return this->res_data;
}

AResponse::~AResponse()
{
    delete this->statuscodes;
}

void AResponse::setCgiBody(const std::string& body)
{
    this->cgi_body = body;
    this->is_cgi_response = true;
    this->res_data.clength = body.length();
}

std::string AResponse::getStatusMessage(int code)
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
                case 502: return "Bad Gateway";
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

void AResponse::addHeader(const std::string& key, const std::string& value)
{
    this->cgi_headers[key] = value;
}

void AResponse::setStatusCode(int code)
{
    this->status_code = code;
}

void AResponse::setContentType(const std::string& contentType)
{
    this->res_data.contentType = contentType;
    this->cgi_headers["Content-Type"] = contentType;
}
    
    /**
     * Get the CGI body content
     * @return CGI response body
     */
const std::string& AResponse::getCgiBody() const
{
    return this->cgi_body;
}
    
    /**
     * Get CGI headers
     * @return Map of CGI headers
     */
const std::map<std::string, std::string>& AResponse::getCgiHeaders() const
{
    return this->cgi_headers;
}
    
    /**
     * Get HTTP status code
     * @return HTTP status code
     */
int AResponse::getStatusCode() const
{
    return this->status_code;
}

bool AResponse::isCgiResponse() const
{
    return this->is_cgi_response;
}
    
void AResponse::setBody(const std::string& body)
{
    if (this->is_cgi_response) {
            setCgiBody(body);
        } else {
            this->response.str("");
            this->response << body;
            this->res_data.clength = body.length();
        }
}
    

std::string AResponse::buildCgiResponse()
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
        
        response_stream << "\r\n";
        
        response_stream << cgi_body;
        
        return response_stream.str();
    }
    

void AResponse::resetCgiData()
{
    this->cgi_body.clear();
    this->cgi_headers.clear();
    this->status_code = 200;
    this->is_cgi_response = false;
    this->res_data.clength = 0;
}
    

bool AResponse::hasCgiHeader(const std::string& headerName) const
{
    return cgi_headers.find(headerName) != cgi_headers.end();
}
    
 
std::string AResponse::getCgiHeader(const std::string& headerName) const
{
    std::map<std::string, std::string>::const_iterator it = cgi_headers.find(headerName);
    if (it != cgi_headers.end()) {
        return it->second;
    }
    return "";
}
 
void AResponse::mergeCgiResponse()
{
    if (is_cgi_response) {
            std::string full_response = buildCgiResponse();
            this->response.str("");
            this->response << full_response;
            this->resp_msg = this->response.str().c_str();
        }
}