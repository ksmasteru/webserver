#include <string>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    char buffer[1024] = {0};
   int socket_fd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(1234);
    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
    connect(socket_fd, (struct sockaddr *)&address, sizeof(address));
    send(socket_fd, "hello", 6, 0);
    printf ("Message sent to server.\n");
    read(socket_fd, buffer, 1024);
    printf("Server response: %s\n", buffer);
    close(socket_fd);

    return 0;
}
