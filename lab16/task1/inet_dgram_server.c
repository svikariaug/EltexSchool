#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 256

int main() {
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }
    
    printf("UDP Server waiting on port %d...\n", PORT);
    
    while (1) {
        client_len = sizeof(client_addr);
        memset(buffer, 0, BUFFER_SIZE);
        
        recvfrom(server_fd, buffer, BUFFER_SIZE, 0,
                 (struct sockaddr*)&client_addr, &client_len);
        
        printf("Received: %s\n", buffer);
        sendto(server_fd, "hi!", strlen("hi!") + 1, 0,
               (struct sockaddr*)&client_addr, client_len);
        printf("Sent: hi!\n");
    }
    
    close(server_fd);
    return 0;
}