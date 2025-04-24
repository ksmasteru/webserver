#pragma once
#include <string>
#include <map>
#pragma once
class  Request{
    private:
    std::string requestPath;
    std::string RawRequest;
    std::string type;
    Request(Request& rhs);
    Request& operator=(Request& rhs);
    std::map<int, std::string> parse_map;
    
    public:
    Request(char *buffer);
    const char* getRequestPath();
    const std::string& getRawRequest() const;
    const std::string& getType() const;
    void parseRequest(char* request);
    std::string getMapAtIndex(unsigned int index);
    ~Request(){}
};

// reponse should take a request ptr ?