#include "Cgi_handler.hpp"

Cgi::Cgi()
    : envp(NULL), pid(0), timeout(DEFAULT_CGI_TIMEOUT)
{
    fds[0] = fds[1] = -1;
}

Cgi::Cgi(Request *req, AResponse *res, const std::string& script_path, int timeout)
    : req(req), res(res), scriptPath(script_path), envp(NULL), pid(0), timeout(timeout)
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
        for (int i = 0; envp[i]; ++i)
            free(envp[i]);
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
    // Empty, used to interrupt `waitpid`
}

void Cgi::load_into_envp()
{
    std::vector<std::string> envs;
    envs.push_back("REQUEST_METHOD=" + req->getType());

    std::string path = req->getfullpath();
    std::cout << "full path" << path << std::endl;
    std::string query;
    if (path.find('?') != std::string::npos)
        query = path.substr(path.find('?') + 1);
    std::cout << "full query" << query<< std::endl;

    envs.push_back("QUERY_STRING=" + query);
    envs.push_back("SCRIPT_NAME=" + scriptPath);
    envs.push_back("SCRIPT_FILENAME=" + scriptPath);
    envs.push_back("SERVER_PROTOCOL=" + req->getHttpVersion());
    envs.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envs.push_back("SERVER_SOFTWARE=WebServer/1.0");

    // Fix: Get content length from request, not response
    std::ostringstream oss;
    if (req->getType() == "POST") {
        
        oss << this->res->getResData().clength; 
    } else {
        oss << "0";
    }
    envs.push_back("CONTENT_LENGTH=" + oss.str());

    oss << this->res->getResData().contentType;
    envs.push_back("CONTENT_TYPE=" + oss.str());



    for (const auto& [k, v] : req->getHeaders()) {
        std::string key = k;
        for (size_t i = 0; i < key.size(); ++i)
            key[i] = (key[i] == '-') ? '_' : std::toupper(key[i]);
        envs.push_back("HTTP_" + key + "=" + v);
    }

    envp = new char *[envs.size() + 1];
    for (size_t i = 0; i < envs.size(); ++i)
        envp[i] = strdup(envs[i].c_str());
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
            interpreter.clear(); // Direct execution
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
        int postFd = req->getPostFd();
        if (postFd != -1)
            dup2(postFd, STDIN_FILENO);
    }

    if (!interpreter.empty()) {
        char* args[] = { const_cast<char*>(interpreter.c_str()), const_cast<char*>(scriptPath.c_str()), NULL };
        execve(interpreter.c_str(), args, envp);
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

    signal(SIGALRM, handle_timeout);
    alarm(timeout);

    int status;
    int ret = waitpid(pid, &status, 0);
    alarm(0);

    if (ret == -1) {
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
        throw std::runtime_error("CGI script timeout or error");
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        throw std::runtime_error("CGI script exited with error");

    if (WIFSIGNALED(status))
        throw std::runtime_error("CGI script killed by signal");
}

void Cgi::handle_cgi_output()
{
    if (fds[0] == -1)
        throw std::runtime_error("No CGI output");

    int flags = fcntl(fds[0], F_GETFL, 0);
    fcntl(fds[0], F_SETFL, flags | O_NONBLOCK);

    std::string output;
    char buf[4096];
    ssize_t r;

    while ((r = read(fds[0], buf, sizeof(buf))) > 0)
        output.append(buf, r);

    if (r == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
        throw std::runtime_error("CGI read error");

    close(fds[0]);
    fds[0] = -1;

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
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

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