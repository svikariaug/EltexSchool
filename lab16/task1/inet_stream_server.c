#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8081
#define BUFFER_SIZE 256

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }
    
    listen(server_fd, 5);
    printf("TCP Server waiting on port %d...\n", PORT);
    
    while (1) {
        client_fd = accept(server_fd, NULL, NULL);
        
        memset(buffer, 0, BUFFER_SIZE);
        read(client_fd, buffer, BUFFER_SIZE);
        printf("Received: %s\n", buffer);
        
        write(client_fd, "hi!", strlen("hi!") + 1);
        printf("Sent: hi!\n");
        
        close(client_fd);
    }
    
    close(server_fd);
    return 0;
}