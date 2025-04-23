#include "../includes/Request.hpp"

const std::map<int, std::string>&   Request::getMap(){
    return (this->parse_map);
}
const std::string&   Request::getRawRequest(){
    return (this->RawRequest);
}
const std::string&   Request::getType(){
    return (this->type);
}

// to avoid copying the map each time. 
const std::string& Request::getMapAtIndex(unsigned int index)
{
    return (this->parse_map[index]);
}