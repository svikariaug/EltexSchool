#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/local_dgram_socket"
#define CLIENT_SOCKET_PATH "/tmp/local_dgram_client_socket"
#define BUFFER_SIZE 256

int main() {
    int client_fd;
    struct sockaddr_un server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    
    // Удаляем старый клиентский сокет если существует
    unlink(CLIENT_SOCKET_PATH);
    
    client_fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (client_fd < 0) {
        perror("socket");
        exit(1);
    }
    
    // Привязываем клиент к своему адресу
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sun_family = AF_LOCAL;
    strcpy(client_addr.sun_path, CLIENT_SOCKET_PATH);
    
    if (bind(client_fd, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0) {
        perror("bind");
        close(client_fd);
        exit(1);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_LOCAL;
    strcpy(server_addr.sun_path, SOCKET_PATH);
    
    strcpy(buffer, "hello!");
    
    printf("Sending: %s\n", buffer);
    
    if (sendto(client_fd, buffer, strlen(buffer) + 1, 0,
               (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("sendto");
        close(client_fd);
        unlink(CLIENT_SOCKET_PATH);
        exit(1);
    }
    
    printf("Message sent, waiting for response...\n");
    
    memset(buffer, 0, BUFFER_SIZE);
    
    if (recvfrom(client_fd, buffer, BUFFER_SIZE, 0, NULL, NULL) < 0) {
        perror("recvfrom");
        close(client_fd);
        unlink(CLIENT_SOCKET_PATH);
        exit(1);
    }
    
    printf("Server response: %s\n", buffer);
    
    close(client_fd);
    unlink(CLIENT_SOCKET_PATH);
    return 0;
}