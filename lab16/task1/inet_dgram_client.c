#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 256

int main() {
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    
    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_fd < 0) {
        perror("socket");
        exit(1);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    
    strcpy(buffer, "hello!");
    
    sendto(client_fd, buffer, strlen(buffer) + 1, 0,
           (struct sockaddr*)&server_addr, sizeof(server_addr));
    
    memset(buffer, 0, BUFFER_SIZE);
    recvfrom(client_fd, buffer, BUFFER_SIZE, 0, NULL, NULL);
    
    printf("Server response: %s\n", buffer);
    
    close(client_fd);
    return 0;
}