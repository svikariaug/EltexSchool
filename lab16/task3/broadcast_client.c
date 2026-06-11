#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BROADCAST_PORT 8087
#define BUFFER_SIZE 256

int main() {
    int client_fd;
    struct sockaddr_in broadcast_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len;
    
    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_fd < 0) {
        perror("socket");
        exit(1);
    }
    
    int broadcast_permission = 1;
    if (setsockopt(client_fd, SOL_SOCKET, SO_BROADCAST,
                   &broadcast_permission, sizeof(broadcast_permission)) < 0) {
        perror("setsockopt");
        exit(1);
    }
    
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(BROADCAST_PORT);
    // Универсальный broadcast адрес
    broadcast_addr.sin_addr.s_addr = INADDR_BROADCAST;
    
    printf("Broadcast Client\n");
    printf("Enter message to broadcast (or 'quit' to exit):\n");
    
    while (1) {
        printf("> ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        
        if (strcmp(buffer, "quit") == 0) {
            break;
        }
        
        if (sendto(client_fd, buffer, strlen(buffer) + 1, 0,
                   (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
            perror("sendto");
            continue;
        }
        
        printf("Broadcast sent: %s\n", buffer);
        
        // Ждем ответ
        struct timeval tv;
        tv.tv_sec = 3;
        tv.tv_usec = 0;
        setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        
        addr_len = sizeof(broadcast_addr);
        memset(buffer, 0, BUFFER_SIZE);
        
        int ret = recvfrom(client_fd, buffer, BUFFER_SIZE, 0,
                           (struct sockaddr*)&broadcast_addr, &addr_len);
        if (ret > 0) {
            printf("Server response: %s\n", buffer);
        } else {
            printf("No response (timeout or server not running)\n");
        }
    }
    
    close(client_fd);
    return 0;
}