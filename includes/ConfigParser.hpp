#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <vector>
#include <map>
#include <string>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "Location.hpp"
#include <iostream>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include "webserver.hpp"
#include "server.hpp"
#include "Location.hpp"
#include "utils.hpp"

class ConfigParser
{
    private:
        std::vector<Server> _servers;

    public:
void printConfig() const;
void parseServerBlock(std::ifstream &conf);
bool startsWithDirective(const std::string &line, const std::string &directive);
void parseLocationBlock(std::ifstream &conf, Location &location);
std::vector<Server> &getServers();
void parse(const std::string &filename);

};

#endif