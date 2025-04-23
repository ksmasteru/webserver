#pragma once
#include <string>
#include <map>
#pragma once
class  Request{
    private:
        std::map<int, std::string>& parse_map;
        std::string& RawRequest;
        std::string type;
        Request(Request& rhs);
        Request& operator=(Request& rhs);
  
    public:
        Request(std::map<int, std::string>& p_map,
            std::string&RequestBuffer) : parse_map(p_map),
                 RawRequest(RequestBuffer){}
        const std::map<int, std::string>& getMap();
        const std::string& getRawRequest();
        const std::string& getType();
        std::string& getMapAtIndex(unsigned int index) const;
        ~Request(){}
};
