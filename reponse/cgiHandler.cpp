#include "../includes/cgiHandler.hpp"
#include <cstdlib>

Cgi::Cgi()
    : envp(NULL), pid(0), timeout(DEFAULT_CGI_TIMEOUT)
{
    fds[0] = fds[1] = -1;
}

Cgi::Cgi(Request *req, AResponse *res, const std::string& script_path, int timeout)
    : envp(NULL), pid(0), req(req), res(res), scriptPath(script_path), timeout(timeout)
{
    fds[0] = fds[1] = -1;
    determine_interpreter();
}

Cgi::~Cgi()
{
    cleanup_resources();
}

void Cgi::cleanup_resources()
{
    if (envp) {
        delete[] envp;
        envp = NULL;
    }
    if (fds[0] != -1)
        close(fds[0]);
    if (fds[1] != -1)
        close(fds[1]);
}

void Cgi::handle_timeout(int)
{
}

void Cgi::load_into_envp()
{
    std::cout << "content type this is  " << this->res->getResData().contentType << std::endl;

    std::vector<std::string> envs;
    envs.push_back("REQUEST_METHOD=" + req->getType());
    
    std::string path = req->getfullpath();
    std::string query;
    size_t queryPos = path.find('?');
    if (queryPos != std::string::npos) {
        std::string rawQuery = path.substr(queryPos + 1);
        size_t httpPos = rawQuery.find(" HTTP/");
        if (httpPos != std::string::npos) {
            query = rawQuery.substr(0, httpPos);
        } else {
            size_t newlinePos = rawQuery.find('\n');
            if (newlinePos != std::string::npos) {
                query = rawQuery.substr(0, newlinePos);
            } else {
                query = rawQuery;
            }
        }
    }
    
    std::cout << "parsed query: " << query << std::endl;
        
    envs.push_back("QUERY_STRING=" + query);
    envs.push_back("SCRIPT_NAME=" + path);
    envs.push_back("SCRIPT_FILENAME=" + scriptPath);
    envs.push_back("SERVER_PROTOCOL=" + req->getHttpVersion());
    envs.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envs.push_back("SERVER_SOFTWARE=WebServer/1.0");
    
    envs.push_back("PYTHONDONTWRITEBYTECODE=1");
    envs.push_back("PYTHONUNBUFFERED=1");
    envs.push_back("PYTHONIOENCODING=utf-8");
    envs.push_back("LC_ALL=C.UTF-8");
    envs.push_back("LANG=C.UTF-8");
    
    envs.push_back("PATH=/usr/local/bin:/usr/bin:/bin");
    
    std::ostringstream contentLengthStream;
    if (req->getType() == "POST") {
        contentLengthStream << this->res->getResData().clength;
    } else {
        contentLengthStream << "0";
    }
    envs.push_back("CONTENT_LENGTH=" + contentLengthStream.str());
    
    std::ostringstream contentTypeStream;
    envs.push_back("CONTENT_TYPE=" + contentTypeStream.str());
    
    std::map<std::string, std::string> headers = req->getHeaders();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); 
         it != headers.end(); ++it)
    {
        std::string key = it->first;
        const std::string& value = it->second;
        
        for (size_t i = 0; i < key.size(); ++i) {
            key[i] = (key[i] == '-') ? '_' : std::toupper(key[i]);
        }
        envs.push_back("HTTP_" + key + "=" + value);
    }
    
    envp = new char *[envs.size() + 1];
    for (size_t i = 0; i < envs.size(); ++i) {
        envp[i] = strdup(envs[i].c_str());
    }
    envp[envs.size()] = NULL;
}

void Cgi::determine_interpreter()
{
    size_t dot = scriptPath.find_last_of('.');
    if (dot != std::string::npos) {
        std::string ext = scriptPath.substr(dot + 1);
        if (ext == "py") interpreter = "/usr/bin/python3";
        else if (ext == "php") interpreter = "/usr/bin/php";
        else if (ext == "pl") interpreter = "/usr/bin/perl";
        else if (ext == "rb") interpreter = "/usr/bin/ruby";
        else if (ext == "sh") interpreter = "/bin/bash";
        else interpreter.clear();
    } else {
        if (access(scriptPath.c_str(), X_OK) == 0) {
            interpreter.clear(); 
        }
    }
}

void Cgi::execute_cgi()
{
    if (pipe(fds) == -1)
        throw std::runtime_error("pipe() failed");

    load_into_envp();
    pid = fork();

    if (pid == -1) {
        cleanup_resources();
        throw std::runtime_error("fork() failed");
    }

    if (pid == 0)
        execute_child_process();
    else
        execute_parent_process();
}

void Cgi::execute_child_process()
{
    close(fds[0]);
    dup2(fds[1], STDOUT_FILENO);
    close(fds[1]);

    if (req->getType() == "POST") {
        //int postFd = req->getPostFd();
        // ???
        int postFd = req->RequestFile.fd;
        if (postFd != -1)
            dup2(postFd, STDIN_FILENO);
    }
    
    if (!interpreter.empty()) {
        if (interpreter.find("python") != std::string::npos) {
            char* args[] = { 
                const_cast<char*>(interpreter.c_str()), 
                const_cast<char*>("-S"),  
                const_cast<char*>("-u"),  
                const_cast<char*>(scriptPath.c_str()), 
                NULL 
            };
            execve(interpreter.c_str(), args, envp);
        } else {
            char* args[] = { 
                const_cast<char*>(interpreter.c_str()), 
                const_cast<char*>(scriptPath.c_str()), 
                NULL 
            };
            execve(interpreter.c_str(), args, envp);
        }
    } else {
        char* args[] = { const_cast<char*>(scriptPath.c_str()), NULL };
        execve(scriptPath.c_str(), args, envp);
    }

    exit(1);
}

void Cgi::execute_parent_process()
{
    close(fds[1]);
    fds[1] = -1;
    int status;
    time_t start = time(NULL);

    while (true)
    {
        int ret = waitpid(pid, &status, WNOHANG);

        if (ret == -1) {
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);
            throw std::runtime_error("CGI script error during execution");
        } else if (ret == 0) {
            if (time(NULL) - start > this->timeout) {
                kill(pid, SIGKILL);
                waitpid(pid, &status, 0);
                throw std::runtime_error("CGI script timeout");
            }
            usleep(100000); 
        } else {
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
                throw std::runtime_error("CGI script exited with error");

            if (WIFSIGNALED(status))
                throw std::runtime_error("CGI script killed by signal");

            break; 
        }
    }
}

void Cgi::handle_cgi_output()
{
    if (fds[0] == -1)
        throw std::runtime_error("No CGI output");

    std::string output;
    char buf[4096];
    ssize_t r;
    bool hasData = false;

    while ((r = read(fds[0], buf, sizeof(buf))) > 0) {
        output.append(buf, r);
        hasData = true;
    }

    if (r == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
        std::cout << "CGI read error: " << strerror(errno) << std::endl;
        throw std::runtime_error("CGI read error");
    }

    close(fds[0]);
    fds[0] = -1;

    if (!hasData) {
        std::cout << "No CGI output received" << std::endl;
        throw std::runtime_error("No CGI output");
    }

    std::cout << "CGI output received: " << output.length() << " bytes" << std::endl;
    parse_cgi_output(output);
}

void Cgi::parse_cgi_output(const std::string& output)
{
    size_t pos = output.find("\r\n\r\n");
    if (pos == std::string::npos)
        pos = output.find("\n\n");

    if (pos == std::string::npos) {
        res->setCgiBody(output);
        return;
    }

    std::string headers = output.substr(0, pos);
    std::string body = output.substr(pos + (output[pos] == '\r' ? 4 : 2));

    std::istringstream stream(headers);
    std::string line;

    while (std::getline(stream, line)) {
        if (!line.empty() && line[line.size()-1] == '\r')
            line = line.substr(0, line.size()-1);

        size_t delim = line.find(':');
        if (delim != std::string::npos) {
            std::string key = line.substr(0, delim);
            std::string val = line.substr(delim + 1);
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            val.erase(0, val.find_first_not_of(" \t"));
            val.erase(val.find_last_not_of(" \t") + 1);

            if (key == "Status")
                res->setStatusCode(atoi(val.c_str()));
            else if (key == "Content-Type")
                res->setContentType(val);
            else
                res->addHeader(key, val);
        }
    }

    res->setCgiBody(body);
}