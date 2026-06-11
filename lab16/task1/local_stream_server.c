#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/local_stream_socket"
#define BUFFER_SIZE 256

int main() {
    int server_fd, client_fd;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];
    
    unlink(SOCKET_PATH);
    
    server_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
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
    
    listen(server_fd, 5);
    printf("Local STREAM Server waiting...\n");
    
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
    unlink(SOCKET_PATH);
    return 0;
}