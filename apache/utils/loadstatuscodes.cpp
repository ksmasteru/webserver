#include "../includes/webserver.hpp"
// fills the server own response map
void loadstatuscodes(const char* filepath)
{
    std::ifstream ifs(filepath);
    if (!ifs)
    {
        std::cerr << "couln't load status codes file " << std::endl; 
        return ;
    }
    std::map<std::string, std::string> map; // use this->map
    std::string line;
    std::string key;
    std::string value;
    while (std::getline(ifs, line))
    {
        key = line.substr(0, 3);
        value = line.substr(5, sizeof(line) - 5);
        map.insert(std::pair<std::string, std::string>(key, value));
    }
    std::map<std::string, std::string>::iterator it;
    for (it = map.begin(); it != map.end(); ++it)
    {
        std::cout << "key is " << it->first << " value is: " << it->second << std::endl;
    }
}