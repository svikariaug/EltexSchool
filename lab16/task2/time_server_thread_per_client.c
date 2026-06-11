#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/socket.h>

#define PORT 8082
#define BUFFER_SIZE 256

void* handle_client(void* arg) {
    int client_fd = *(int*)arg;
    char buffer[BUFFER_SIZE];
    time_t now;
    
    free(arg);
    
    memset(buffer, 0, BUFFER_SIZE);
    read(client_fd, buffer, BUFFER_SIZE);
    printf("Request from client\n");
    
    now = time(NULL);
    strcpy(buffer, ctime(&now));
    write(client_fd, buffer, strlen(buffer) + 1);
    
    close(client_fd);
    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr;
    pthread_t thread;
    
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
    printf("Time Server (Thread per client) on port %d\n", PORT);
    
    while (1) {
        client_fd = accept(server_fd, NULL, NULL);
        
        int* client_fd_ptr = malloc(sizeof(int));
        *client_fd_ptr = client_fd;
        pthread_create(&thread, NULL, handle_client, client_fd_ptr);
        pthread_detach(thread);
    }
    
    close(server_fd);
    return 0;
}