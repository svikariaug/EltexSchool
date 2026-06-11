#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>
#include <sys/socket.h>

#define TCP_PORT 8085
#define UDP_PORT 8085
#define BUFFER_SIZE 256

int main() {
    int tcp_fd, udp_fd, max_fd;
    struct sockaddr_in tcp_addr, udp_addr, client_addr;
    socklen_t client_len;
    fd_set read_fds;
    char buffer[BUFFER_SIZE];
    time_t now;
    
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (tcp_fd < 0 || udp_fd < 0) {
        perror("socket");
        exit(1);
    }
    
    int opt = 1;
    setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(udp_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&tcp_addr, 0, sizeof(tcp_addr));
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_addr.s_addr = INADDR_ANY;
    tcp_addr.sin_port = htons(TCP_PORT);
    
    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(UDP_PORT);
    
    bind(tcp_fd, (struct sockaddr*)&tcp_addr, sizeof(tcp_addr));
    bind(udp_fd, (struct sockaddr*)&udp_addr, sizeof(udp_addr));
    
    listen(tcp_fd, 5);
    
    printf("Multiprotocol Server on port %d (TCP and UDP)\n", TCP_PORT);
    
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(tcp_fd, &read_fds);
        FD_SET(udp_fd, &read_fds);
        
        max_fd = (tcp_fd > udp_fd) ? tcp_fd : udp_fd;
        
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(1);
        }
        
        if (FD_ISSET(tcp_fd, &read_fds)) {
            int client_fd = accept(tcp_fd, NULL, NULL);
            
            memset(buffer, 0, BUFFER_SIZE);
            read(client_fd, buffer, BUFFER_SIZE);
            
            now = time(NULL);
            strcpy(buffer, ctime(&now));
            write(client_fd, buffer, strlen(buffer) + 1);
            
            close(client_fd);
            printf("Served TCP client\n");
        }
        
        if (FD_ISSET(udp_fd, &read_fds)) {
            client_len = sizeof(client_addr);
            memset(buffer, 0, BUFFER_SIZE);
            
            recvfrom(udp_fd, buffer, BUFFER_SIZE, 0,
                     (struct sockaddr*)&client_addr, &client_len);
            
            now = time(NULL);
            strcpy(buffer, ctime(&now));
            sendto(udp_fd, buffer, strlen(buffer) + 1, 0,
                   (struct sockaddr*)&client_addr, client_len);
            
            printf("Served UDP client\n");
        }
    }
    
    close(tcp_fd);
    close(udp_fd);
    return 0;
}