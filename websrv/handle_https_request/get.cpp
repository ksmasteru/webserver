#include "../includes/webserver.hpp"

char buffer[8192];
ssize_t bytes_read;
char chunk_size[16];

int send_all(int fd, const char *buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t sent = send(fd, buf + total, len - total, 0);
        std::cout << " inside the function " << sent << "and the total is " << total  <<std::endl;
        if (sent == -1) return -1; 
        total += sent;
    }
    return 0;
}

std::string content(std::string extension )
{
   std::map<std::string, std::string> contentMap = {
        {"html", "text/html"},
        {"htm",  "text/html"},
        {"css",  "text/css"},
        {"js",   "application/javascript"},
        {"json", "application/json"},
        {"png",  "image/png"},
        {"jpg",  "image/jpg"},
        {"jpeg", "image/jpeg"},
        {"gif",  "image/gif"},
        {"svg",  "image/svg+xml"},
        {"txt",  "text/plain"},
        {"mp4", "text/html"},
        {"", "text/plain"}
    };
    return contentMap[extension];
}

std::string RspStatusline(unsigned int code)
{
    // HTTP/{VERSION(DOUBLE)} {STATUS CODE} RESPONSE
    // HTTP/1.1 200 OK
    // map response to code.
    std::string http_version = "HTTP/1.1";
    std::string statusCode = intToString(code);
    std::string Response = "OK"; // reponse message.
    std::string rsp = http_version + " " + statusCode + " " + Response + "\r\n";
    return (rsp);
}

std::string getTime()
{
    std::time_t now = std::time(nullptr);
    std::tm *gmt = std::gmtime(&now);  // Get GMT time

    char buffer[100];
    // Format: "Mon, 22 Apr 2025 10:00:00 GMT"
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmt);
    return (buffer);
    
}

std::string RspHeader(unsigned int cLength, bool keepAlive)
{
    // make this work with a struct  :
    // date:
    // server:
    // content-type:
    // content-length:
    std::string linebuf = "\r\n";
    std::string getCurrentTime = "date: " + getTime() + linebuf;
    std::string server =  "server: apache/2.4.41 (Ubuntu)" + linebuf;
    std::string contentType = "content-Type: text/html; charset=UTF-8" + linebuf;
    std::string contentLength =  "content-Length: " + intToString(cLength)+ linebuf;
    std::string alive = (keepAlive ? "keep-alive" : "close") + linebuf;
    std::string res = getCurrentTime + server + contentType + contentLength + "Connection: " + alive;
    return res;
}

std::string makeRspHeader(unsigned int status, unsigned int contentLength , bool keepAlive)
{
  /*  std::string header = "HTTP/1.1 402 File not found \r\n"
                                   "Content-Type: text/html\r\n"
                                   "Connection: close\r\n"
                                   "\r\n"; */
    std::string header = RspStatusline(status) + RspHeader(contentLength, keepAlive);
    return (header);
}

void handle_get(int client_fd, std::map<int, struct client> *activity , std::map<int, std::string> map)
{
    std::ifstream file("./reponse/403.html"); // reponse body. ?
    std::ifstream file2("./reponse/403.html");
    std::string resp_buff;
    std::stringstream response_buffer(resp_buff);
    response_buffer << file.rdbuf();
    // basically : made our response_buffer content that of ifstream file2.
    std::string html_content = response_buffer.str();
    std::string path = "./static_files" + map[0];
    struct client new_client;
    new_client.start = 0;
    // response KO
    struct resp_h resp;
    if (access(path.c_str(), F_OK) == -1)
    {
        response_buffer << file.rdbuf();
        std::string html_content = response_buffer.str();
        std::string response = "HTTP/1.1 402 File not found \r\n"
                                   "Content-Type: text/html\r\n"
                                   "Connection: close\r\n"
                                   "\r\n" +
                                   html_content;
            write(client_fd, response.c_str(), response.length());
            close(client_fd);
    }
    else if (access(path.c_str(), R_OK) == -1)
    {
        response_buffer << file2.rdbuf();
        std::string html_content = response_buffer.str();
        std::string response = "HTTP/1.1 403 Forbidden  \r\n"
                                   "Content-Type: text/html\r\n"
                                   "Connection: close\r\n"
                                   "\r\n" +
                                   html_content;
            write(client_fd, response.c_str(), response.length());
            close(client_fd);
    }
        else // response. :: only here is handled in ::getResponse. f
        {
            std::ifstream file1;
            std::string extension = "";
            std::stringstream body;
            if (map[0] == "/")
            {
                file1.open("./static_files/index.html");
                resp.extension = "text/html";
                
            }
            else
            {
                file1.open(path);
                size_t extension_pos = path.rfind(".");
                resp.extension = (extension_pos != std::string::npos) ? path.substr(extension_pos+1) : "";
            }
            body << file1.rdbuf();
            std::string body_str = body.str();
            std::string content_length = std::to_string(body_str.length());


            struct client check = (*activity)[client_fd];

            resp.keepAlive = false;
            if (map.find(6) != map.end() && map[6].find("keep-alive") != std::string::npos)
                resp.keepAlive = true;
    // testing this.
    /*std::string response = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: "+ (content(extension)) +" \r\n"
                       "Content-Length: " + content_length + "\r\n"
                       "Connection: " + (keep_alive ? "keep-alive" : "close") + "\r\n"
                       "\r\n" +
                       body_str;*/
    std::string response = makeRspHeader(200, body_str.length(), resp.keepAlive);
    /*std::cout << "responde header ist " <<std::endl;
    std::cout << response;
    exit(0);*/
    response += body_str;
    if(extension == "mp4")
    {
        std::string chunk_header = "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: video/mp4\r\n"
        "\r\n";
        send(client_fd, chunk_header.c_str(), chunk_header.length(), 0);
        int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening file");
        close(client_fd);
        return;
    }
    
    
while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
    int size_len = snprintf(chunk_size, sizeof(chunk_size), "%zx\r\n", (size_t)bytes_read);
    
    if (send_all(client_fd, chunk_size, size_len) == -1) break;
    std::cout << chunk_size; 
    buffer[bytes_read] = 0;
    if (send_all(client_fd, buffer, bytes_read) == -1) break;
    
    if (send_all(client_fd, "\r\n", 2) == -1) break;

    std::cout << "\n" << "\n" << std::endl;
}
send_all(client_fd, "0\r\n\r\n", 5);

close(fd);
}
else
    send(client_fd, response.c_str(), response.length(), 0);


    if (keep_alive)
    {
        (*activity)[client_fd] = {1, time(NULL)};
        return;
    }
    else
    {
        std::cout << "Closing connection " << client_fd << " (not keep-alive)" << std::endl;
        close(client_fd);
        return;
    }

        }
    }
    
