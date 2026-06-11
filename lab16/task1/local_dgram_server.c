#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/local_dgram_socket"
#define BUFFER_SIZE 256

int main() {
    int server_fd;
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    
    unlink(SOCKET_PATH);
    
    server_fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_LOCAL;
    strcpy(server_addr.sun_path, SOCKET_PATH);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }
    
    printf("Local DGRAM Server waiting...\n");
    
    while (1) {
        client_len = sizeof(client_addr);
        memset(buffer, 0, BUFFER_SIZE);
        
        if (recvfrom(server_fd, buffer, BUFFER_SIZE, 0, 
                     (struct sockaddr*)&client_addr, &client_len) < 0) {
            perror("recvfrom");
            continue;
        }
        
        printf("Received: %s\n", buffer);
        
        if (sendto(server_fd, "hi!", strlen("hi!") + 1, 0,
                   (struct sockaddr*)&client_addr, client_len) < 0) {
            perror("sendto");
            continue;
        }
        printf("Sent: hi!\n");
    }
    
    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}