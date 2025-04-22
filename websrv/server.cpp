#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#define BUFFER_SIZE 1024
int main() {
    int serverfd = socket(AF_INET,SOCK_STREAM,0);
    char buffer[BUFFER_SIZE] = {0};
    struct sockaddr_in addr; 
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(serverfd , (struct sockaddr *)&addr,sizeof(addr));
    listen(serverfd,5);
    
    int new_socket = accept(serverfd, (struct sockaddr *)&addr, (socklen_t*)&addr);
    read(new_socket, buffer, BUFFER_SIZE);
        printf("Client message: %s\n", buffer);
        send(new_socket, "client", 7, 0);
    printf("Response sent to client.\n");
        close(new_socket);
        close(serverfd);
    return 0;
}
