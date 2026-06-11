#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MULTICAST_PORT 8088
#define MULTICAST_GROUP "239.0.0.1"
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
    
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        exit(1);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(MULTICAST_PORT);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }
    
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
    mreq.imr_interface.s_addr = INADDR_ANY;
    
    if (setsockopt(server_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt");
        exit(1);
    }
    
    printf("Multicast Server listening on %s:%d\n", MULTICAST_GROUP, MULTICAST_PORT);
    
    while (1) {
        client_len = sizeof(client_addr);
        memset(buffer, 0, BUFFER_SIZE);
        
        recvfrom(server_fd, buffer, BUFFER_SIZE, 0,
                 (struct sockaddr*)&client_addr, &client_len);
        
        printf("Received from %s:%d: %s\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);
        
        char response[BUFFER_SIZE];
        snprintf(response, BUFFER_SIZE, "ACK from server: %.235s", buffer);
        
        sendto(server_fd, response, strlen(response) + 1, 0,
               (struct sockaddr*)&client_addr, client_len);
        
        printf("Sent response to %s:%d\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }
    
    close(server_fd);
    return 0;
}