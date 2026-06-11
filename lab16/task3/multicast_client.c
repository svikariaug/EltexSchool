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
    int client_fd;
    struct sockaddr_in multicast_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len;
    
    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_fd < 0) {
        perror("socket");
        exit(1);
    }
    
    int reuse = 1;
    if (setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        exit(1);
    }
    
    memset(&multicast_addr, 0, sizeof(multicast_addr));
    multicast_addr.sin_family = AF_INET;
    multicast_addr.sin_port = htons(MULTICAST_PORT);
    multicast_addr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);
    
    char message[BUFFER_SIZE];
    printf("Enter message to send via multicast: ");
    fgets(message, BUFFER_SIZE, stdin);
    message[strcspn(message, "\n")] = 0;
    
    if (sendto(client_fd, message, strlen(message) + 1, 0,
               (struct sockaddr*)&multicast_addr, sizeof(multicast_addr)) < 0) {
        perror("sendto");
        exit(1);
    }
    
    printf("Multicast sent to %s:%d\n", MULTICAST_GROUP, MULTICAST_PORT);
    
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
    mreq.imr_interface.s_addr = INADDR_ANY;
    
    if (setsockopt(client_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt (ADD_MEMBERSHIP)");
        exit(1);
    }
    
    addr_len = sizeof(multicast_addr);
    memset(buffer, 0, BUFFER_SIZE);
    
    if (recvfrom(client_fd, buffer, BUFFER_SIZE, 0,
                 (struct sockaddr*)&multicast_addr, &addr_len) > 0) {
        printf("Server response: %s\n", buffer);
    }
    
    close(client_fd);
    return 0;
}