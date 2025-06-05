#ifndef LOCATION_HPP
#define LOCATION_HPP

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

class Location {
private:
    std::string _path;
    std::string _root;
    std::string _index;
    std::string _uploadPath;
    std::string _cgiExtension;
    std::string _cgiPath;
    std::vector<std::string> _allowedMethods;
    std::map<int, std::string> redirections;
    bool _autoindex;

public:
    Location() : _autoindex(false) {}
    
    void setPath(const std::string& path) { _path = path; }
    void setRoot(const std::string& root) { _root = root; }
    void setIndex(const std::string& index) { _index = index; }
    void setUploadPath(const std::string& path) { _uploadPath = path; }
    void setCgiExtension(const std::string& ext) { _cgiExtension = ext; }
    void setCgiPath(const std::string& path) { _cgiPath = path; }
    void setAutoindex(bool autoindex) { _autoindex = autoindex; }
    void addAllowedMethod(const std::string& method) {
        _allowedMethods.push_back(method);
    }
    void addRedirection(int code, const std::string& path) {
        redirections[code] = path;
    }
    
    // Getters
    std::string& getPath() { return _path; }
    std::string& getRoot() { return _root; }
    const std::string& getIndex() const { return _index; }
    const std::string& getUploadPath() const { return _uploadPath; }
    const std::string& getCgiExtension() const { return _cgiExtension; }
    const std::string& getCgiPath() const { return _cgiPath; }
    bool getAutoindex() const { return _autoindex; }
    const std::vector<std::string>& getAllowedMethods() const { return _allowedMethods; }
    std::map<int, std::string> getRedirections() { return redirections; }
    
    // Debug print
    void print() const {
        std::cout << "  Location: " << _path << std::endl;
        std::cout << "    Root: " << _root << std::endl;
        std::cout << "    Index: " << _index << std::endl;
        std::cout << "    Autoindex: " << (_autoindex ? "on" : "off") << std::endl;
        if (!_uploadPath.empty())
            std::cout << "    Upload Path: " << _uploadPath << std::endl;
        if (!_cgiExtension.empty())
            std::cout << "    CGI Extension: " << _cgiExtension << std::endl;
        if (!_cgiPath.empty())
            std::cout << "    CGI Path: " << _cgiPath << std::endl;
        
        if (!_allowedMethods.empty()) {
            std::cout << "    Allowed Methods: ";
            for (const auto& method : _allowedMethods) {
                std::cout << method << " ";
            }
            std::cout << std::endl;
        }
    }
};

#endif