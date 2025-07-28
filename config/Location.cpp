#include "../includes/Location.hpp"
#include <iostream>

Location::Location() : _autoindex(false), redirectBlock(false) {}

void Location::setPath(const std::string& path) { 
    _path = path; 
}

void Location::setRoot(const std::string& root) { 
    _root = root; 
}

void Location::setIndex(const std::string& index) { 
    _index = index; 
}

void Location::setUploadPath(const std::string& path) { 
    _uploadPath = path; 
}

void Location::setCgiExtension(const std::string& ext) { 
    _cgiExtension = ext; 
}

void Location::setCgiPath(const std::string& path) { 
    _cgiPath = path; 
}

void Location::setAutoindex(bool autoindex) { 
    _autoindex = autoindex; 
}

void Location::addAllowedMethod(const std::string& method) {
    _allowedMethods.push_back(method);
}

void Location::addRedirection(int code, const std::string& path) {
    std::cout << "redirection key : " << code << std::endl;
    std::cout << "redirection value : " << path << std::endl;
    redirectBlock = true;
    redirections[code] = path;
}

std::string& Location::getPath() { 
    return _path; 
}

std::string& Location::getRoot() { 
    return _root; 
}

const std::string& Location::getIndex() const { 
    return _index; 
}

const std::string& Location::getUploadPath() const { 
    return _uploadPath; 
}

const std::string& Location::getCgiExtension() const { 
    return _cgiExtension; 
}

const std::string& Location::getCgiPath() const { 
    return _cgiPath; 
}

bool Location::getAutoindex() const { 
    return _autoindex; 
}

const std::vector<std::string>& Location::getAllowedMethods() const { 
    return _allowedMethods; 
}

std::map<int, std::string> Location::getRedirections() { 
    return redirections; 
}

void Location::print() const {
    std::cout << " Location: " << _path << std::endl;
    std::cout << " Root: " << _root << std::endl;
    std::cout << " Index: " << _index << std::endl;
    std::cout << " Autoindex: " << (_autoindex ? "on" : "off") << std::endl;
    if (!_uploadPath.empty())
        std::cout << " Upload Path: " << _uploadPath << std::endl;
    if (!_cgiExtension.empty())
        std::cout << " CGI Extension: " << _cgiExtension << std::endl;
    if (!_cgiPath.empty())
        std::cout << " CGI Path: " << _cgiPath << std::endl;
    if (!_allowedMethods.empty()) {
        std::cout << " Allowed Methods: ";
        for (std::vector<std::string>::const_iterator it = _allowedMethods.begin(); 
             it != _allowedMethods.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
    }
}