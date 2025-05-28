/*
 *   A simple class for webserver
 *
 */
#pragma once

#include "webserver.hpp"
#include "Request.hpp"
#include "AResponse.hpp"
#include "Connection.hpp"
#include "Location.hpp"

class Connection;
class Server
{
private:
    std::vector<int> _ports;
    std::vector<std::string> _hosts;
    std::vector<std::string> _serverNames;
    size_t _clientMaxBodySize;
    std::map<int, std::string> _errorPages;
    std::vector<Location> _locations;
    //
    //
    std::map<std::string, std::string> statusCodes;
    Server(Server &rhs);
    // Server &operator=(Server &rhs);
    bool loadedStatusCodes;
    std::map<int, struct client> activity;
    t_InetData data;
    std::map<int, Connection *> clients;

public:
    Server();
    Server(const Server& obj)
    {
        if (this != &obj)
            *this = obj;
    }
    ~Server(){}
    void setHost(const std::string &host) { _hosts.push_back(host); }
    void setPort(int port)
    {
        if (port > 65535 || port < 1)
            throw std::runtime_error("Error: Invalid port number: " + std::to_string(port));
        _ports.push_back(port);
    }
    void setServerName(const std::string &name) { _serverNames.push_back(name); }
    void setClientMaxBodySize(size_t size) { _clientMaxBodySize = size; }
    void addErrorPage(int code, const std::string &path) { _errorPages[code] = path; }
    void addLocation(const Location &location) { _locations.push_back(location); }

    // Getters
    std::vector<std::string> getHosts() { return _hosts; }
    std::vector<int> getPorts() const { return _ports; }
    std::vector<std::string> &getServerNames() { return _serverNames; }
    size_t getClientMaxBodySize() const { return _clientMaxBodySize; }
    const std::map<int, std::string> &getErrorPages() const { return _errorPages; }
    const std::vector<Location> &getLocations() const { return _locations; }

    void removePort(std::string port)
    {
        for (int i = 0; i < _ports.size(); i++)
        {
            if (_ports[i] == std::stoi(port))
                _ports.erase(_ports.begin() + i);
        }
    }
    void removeHost(std::string host)
    {
        for (int i = 0; i < _hosts.size(); i++)
        {
            if (_hosts[i] == host)
                _hosts.erase(_hosts.begin() + i);
        }
    }

    // Debug print
    void print() const
    {
        std::cout << "Server:" << std::endl;
        for (auto &host : _hosts)
            std::cout << "  Host: " << host << std::endl;
        for (int i = 0; i < _ports.size(); i++)
            std::cout << "  Port: " << _ports[i] << std::endl;
        for (const auto &name : _serverNames)
            std::cout << "  Server Name: " << name << std::endl;
        std::cout << "  Max Body Size: " << _clientMaxBodySize << std::endl;

        if (!_errorPages.empty())
        {
            std::cout << "  Error Pages:" << std::endl;
            for (const auto &page : _errorPages)
            {
                std::cout << "    " << page.first << " -> " << page.second << std::endl;
            }
        }

        if (!_locations.empty())
        {
            std::cout << "  Locations:" << std::endl;
            for (const auto &loc : _locations)
            {
                loc.print();
            }
        }
    }
    //
    //
    void handleReadEvent(int);
    void handleWriteEvent(int);
    void removeClient(int);
    void addNewClient();
    void loadstatuscodes(const char *filepath);
    int establishServer();
    int run();
    void handleRequest(int efd);
    char *getRequest(int client_fd);
    void parseRequest(const std::string &request, std::map<int, std::string> &map);
    void sendBadRequest(int);
};

// handle request : getRequest --> getResponse --> sendRespond