#ifndef CGI_HPP
#define CGI_HPP

#include "../includes/server.hpp"
#include "../includes/Request.hpp"
#include <vector>
#include <cstring>
#include "../includes/AResponse.hpp"
#include "../includes/GetResponse.hpp"

#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <sstream>

class AResponse;
class Request;
class Cgi
{
private:
    char **envp;
    int pid;
    int fds[2];  
    Request *req;
    AResponse *res;
    static const int DEFAULT_CGI_TIMEOUT = 5;
    int timeout;
    std::string scriptPath;
    std::string interpreter;

    static void handle_timeout(int signum);
    void setup_timeout();
    void cleanup_on_timeout();
    void determine_interpreter();
    void cleanup_resources();
    void load_into_envp();
    void execute_child_process();
    void execute_parent_process();
    void parse_cgi_output(const std::string& output);

public:
    Cgi();
    Cgi(Request *req, AResponse *res, const std::string& script_path, int timeout = DEFAULT_CGI_TIMEOUT);
    ~Cgi();

    void execute_cgi();
    void handle_cgi_output();
};

#endif 