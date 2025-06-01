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
#include "server.hpp"
#include "Location.hpp"
#include "utils.hpp"

class ConfigParser {
private:
    std::vector<Server> _servers;

public:
    // Parse the config file
    void parse(const std::string& filename) {
        std::ifstream conf(filename);
        if (!conf.is_open()) {
            throw std::runtime_error("Error: Cannot open config file: " + filename);
        }
        
        std::string line;
        while (getline(conf, line)) {
            trim(line);
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') continue;
            
            if (line.find("server") != std::string::npos && line.find("{") != std::string::npos) {
                parseServerBlock(conf);
            }
        }
        
        if (_servers.empty()) {
            throw std::runtime_error("Error: No server blocks found in config file");
        }
    }
    bool startsWithDirective(const std::string &line, const std::string &directive)
    {
        return line.compare(0, directive.size(), directive) == 0 &&
               (line.size() == directive.size() || isspace(line[directive.size()]));
    }
    // Parse a server block
    void parseServerBlock(std::ifstream& conf) {
        int foundPort = 0;
        int foundHost = 0;
        Server server;
        std::string line;
        bool closingBraceFound = false;
        
        while (getline(conf, line)) {
            trim(line);
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') continue;
            
            if (line == "}") {
                closingBraceFound = true;
                break;
            }
            if (line[line.size() - 1] != ';' && line.find("location") == std::string::npos)
                throw std::runtime_error("Error: Missing semicolon");

            if (startsWithDirective(line, "listen")) {
                if (foundPort == 0)
                    foundPort = 1;
                if (foundPort == 1)
                {
                    server.removePort("80");
                    foundPort = 2;
                }
                std::string port_str = extractValue(line, "listen");
                if (port_str.empty())
                    throw std::runtime_error("Error: Missing port number in listen directive");
                std::stringstream ss(port_str);
                while (getline(ss, port_str, ' '))
                {
                    try
                    {
                        if (port_str.size() > 5)
                            throw 1;
                        server.setPort(std::stoi(port_str));
                    }
                    catch (...)
                    {
                        throw std::runtime_error("Error: Invalid port number: " + port_str);
                    }
                }
            }
            else if (startsWithDirective(line, "host")) {
                if (foundHost == 0)
                    foundHost = 1;
                if (foundHost == 1)
                {
                    server.removeHost("0.0.0.0");
                    foundHost = 2;
                }
                std::string host = extractValue(line, "host");
                if (host.empty())
                    throw std::runtime_error("Error: Missing port number in listen directive");
                std::stringstream ss(host);
                while (getline(ss, host, ' '))
                {
                    try
                    {
                        server.setHost(host);
                    }
                    catch (...)
                    {
                        throw std::runtime_error("Error: Invalid host name: " + host);
                    }
                }
            }
            else if (startsWithDirective(line, "server_name")) {
                std::string name = extractValue(line, "server_name");
                if (name.empty())
                    throw std::runtime_error("Error: Missing port number in listen directive");
                std::stringstream ss(name);
                while (getline(ss, name, ' '))
                {
                    try
                    {
                        server.setServerName(name);
                    }
                    catch (...)
                    {
                        throw std::runtime_error("Error: Invalid server name: " + name);
                    }
                }
            }
            else if (startsWithDirective(line, "client_max_body_size")) {
                std::string size_str = extractValue(line, "client_max_body_size");
                if (size_str.empty())
                    throw std::runtime_error("Error: Missing port number in listen directive");
                
                size_t multiplier = 1;
                char unit = size_str.back();
                if (std::isalpha(unit)) {
                    size_str.pop_back();
                    if (unit == 'k' || unit == 'K') multiplier = 1024;
                    else if (unit == 'm' || unit == 'M') multiplier = 1024 * 1024;
                    else if (unit == 'g' || unit == 'G') multiplier = 1024 * 1024 * 1024;
                }
                
                try {
                    size_t size = std::stoul(size_str) * multiplier;
                    server.setClientMaxBodySize(size);
                } catch (...) {
                    std::cerr << "Error: Invalid client_max_body_size: " << size_str << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            else if (startsWithDirective(line, "error_page")) {
                std::string value = extractValue(line, "error_page");
                if (value.empty())
                    throw std::runtime_error("Error: Missing error_page directive");
                std::vector<std::string> tokens = split(value, ' ');
                
                if (tokens.size() >= 2) {
                    std::string path = tokens.back();
                    
                    // Remove path from tokens
                    tokens.pop_back();
                    
                    // Parse error codes
                    for (const auto& code_str : tokens) {
                        try {
                            int error_code = std::stoi(code_str);
                            server.addErrorPage(error_code, path);
                        } catch (...) {
                            throw ;
                        }
                    }
                }
                else
                    throw std::runtime_error("Error: Invalid error_page directive");
            }
            else if (startsWithDirective(line, "location"))
            {
                if (line.find("{") == std::string::npos || line[line.size() - 1] != '{')
                    throw std::runtime_error("Error: Missing opening brace for location block");
                Location location;
                
                // Extract location path
                size_t path_start = line.find("location") + 8;
                size_t path_end = line.find("{");
                std::string path = line.substr(path_start, path_end - path_start);
                trim(path);
                if (path.empty())
                    throw std::runtime_error("Error: Missing path in location directive");
                location.setPath(path);
                
                // Parse location block
                parseLocationBlock(conf, location);
                
                // Add location to server
                server.addLocation(location);
            }
            else
                throw std::runtime_error("Error: Unknown directive in server block: " + line);
        }
        
        if (!closingBraceFound) {
            throw std::runtime_error("Error: Missing closing brace for server block");
        }
        
        _servers.push_back(server);
    }
    
    // Parse a location block
    void parseLocationBlock(std::ifstream& conf, Location& location) {
        std::string line;
        bool closingBraceFound = false;
        
        while (getline(conf, line)) {
            trim(line);
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') continue;
            
            if (line == "}") {
                closingBraceFound = true;
                break;
            }
            if (line[line.size() - 1] != ';')
                throw std::runtime_error("Error: Missing semicolon");
            if (startsWithDirective(line, "autoindex")) {
                std::string autoindex = extractValue(line, "autoindex");
                if (autoindex != "on" && autoindex != "off")
                    throw std::runtime_error("Error: Invalid value for autoindex directive");
                location.setAutoindex(autoindex == "on");
            }
            else if (startsWithDirective(line, "root")) {
                std::string root = extractValue(line, "root");
                if (root.empty())
                    throw std::runtime_error("Error: Missing root path in location directive");
                location.setRoot(root);
            }
            else if (startsWithDirective(line, "index")) {
                std::string index = extractValue(line, "index");
                if (index.empty())
                    throw std::runtime_error("Error: Missing index file in location directive");
                location.setIndex(index);
            }
            else if (startsWithDirective(line, "upload_path")) {
                std::string upload_path = extractValue(line, "upload_path");
                if (upload_path.empty())
                    throw std::runtime_error("Error: Missing upload path in location directive");
                location.setUploadPath(upload_path);
            }
            else if (startsWithDirective(line, "cgi_extension")) {
                std::string extension = extractValue(line, "cgi_extension");
                if (extension.empty())
                    throw std::runtime_error("Error: Missing CGI extension in location directive");
                location.setCgiExtension(extension);
            }
            else if (startsWithDirective(line, "cgi_path")) {
                std::string cgi_path = extractValue(line, "cgi_path");
                if (cgi_path.empty())
                    throw std::runtime_error("Error: Missing CGI path in location directive");
                location.setCgiPath(cgi_path);
            }
            else if (startsWithDirective(line, "allowed_methods")) {
                std::string methods = extractValue(line, "allowed_methods");
                if (methods.empty())
                    throw std::runtime_error("Error: Missing allowed methods in location directive");
                std::vector<std::string> method_list = split(methods, ' ');
                for (const auto& method : method_list) {
                    // std::cout << "Allowed method: " << method << std::endl;
                    if (method != "GET" && method != "POST" && method != "DELETE")
                        throw std::runtime_error("Error: Invalid HTTP method in allowed_methods directive");
                    location.addAllowedMethod(method);
                }
            }
            else if (startsWithDirective(line, "return")) {
                std::string value = extractValue(line, "return");
                if (value.empty())
                    throw std::runtime_error("Error: Missing return directive");
                std::vector<std::string> tokens = split(value, ' ');
                
                if (tokens.size() >= 2) {
                    std::string path = tokens.back();
                    
                    tokens.pop_back();
                    
                    for (const auto& code_str : tokens) {
                        try {
                            int error_code = std::stoi(code_str);
                            if (error_code != 301)
                                throw std::runtime_error("Error: Invalid HTTP status code in return directive");
                            location.addRedirection(error_code, path);
                        } catch (...) {
                            throw ;
                        }
                    }
                }
                else
                    throw std::runtime_error("Error: Invalid return directive");
            }
            else
                throw std::runtime_error("Error: Unknown directive in location block: " + line);
        }
        
        if (!closingBraceFound)
            throw std::runtime_error("Error: Missing closing brace for location block");
    }
    
    // Get all parsed servers
    std::vector<Server>& getServers() {
        return _servers;
    }
    
    // Debug print all servers
    void printConfig() const {
        std::cout << "Configuration contains " << _servers.size() << " server(s):" << std::endl;
        for (const auto& server : _servers) {
            server.print();
            std::cout << "------------------------" << std::endl;
        }
    }
};

#endif