#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <vector>
#include <map>
#include <string>
#include <iostream>

class Location {
private:
    std::string _path;
    std::string _root;
    std::string _index;
    std::string _uploadPath;
    std::string _cgiExtension;
    std::string _cgiPath;
    std::vector<std::string> _allowedMethods;
    bool _autoindex;

public:
    bool redirectBlock;
    std::map<int, std::string> redirections;

    // Constructor
    Location();

    // Setters
    void setPath(const std::string& path);
    void setRoot(const std::string& root);
    void setIndex(const std::string& index);
    void setUploadPath(const std::string& path);
    void setCgiExtension(const std::string& ext);
    void setCgiPath(const std::string& path);
    void setAutoindex(bool autoindex);
    void addAllowedMethod(const std::string& method);
    void addRedirection(int code, const std::string& path);

    // Getters
    std::string& getPath();
    std::string& getRoot();
    const std::string& getIndex() const;
    const std::string& getUploadPath() const;
    const std::string& getCgiExtension() const;
    const std::string& getCgiPath() const;
    bool getAutoindex() const;
    const std::vector<std::string>& getAllowedMethods() const;
    std::map<int, std::string> getRedirections();

    // Debug print
    void print() const;
};

#endif