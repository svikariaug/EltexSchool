#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#define SERVER_PORT 8888
#define BUFFER_SIZE 65536

int client_sock;
int client_port;
uint32_t server_ip;
int shutdown_sent = 0;

uint16_t checksum(uint16_t *buf, int len) {
    uint32_t sum = 0;
    while (len > 1) {
        sum += *buf++;
        len -= 2;
    }
    if (len) sum += *(uint8_t*)buf;
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);
    return (uint16_t)(~sum);
}

void compute_ip_checksum(struct iphdr *ip) {
    ip->check = 0;
    ip->check = checksum((uint16_t*)ip, ip->ihl * 4);
}

void compute_udp_checksum(struct iphdr *ip, struct udphdr *udp,
                          uint8_t *data, int data_len) {
    struct pseudo_hdr {
        uint32_t src;
        uint32_t dst;
        uint8_t  zero;
        uint8_t  proto;
        uint16_t len;
    } pseudo;

    pseudo.src  = ip->saddr;
    pseudo.dst  = ip->daddr;
    pseudo.zero = 0;
    pseudo.proto = ip->protocol;
    pseudo.len  = htons(sizeof(struct udphdr) + data_len);

    int udp_len = sizeof(struct udphdr) + data_len;
    unsigned char *buffer = malloc(sizeof(pseudo) + udp_len);
    memcpy(buffer, &pseudo, sizeof(pseudo));
    memcpy(buffer + sizeof(pseudo), udp, sizeof(struct udphdr));
    memcpy(buffer + sizeof(pseudo) + sizeof(struct udphdr), data, data_len);

    struct udphdr *udp_copy = (struct udphdr*)(buffer + sizeof(pseudo));
    udp_copy->check = 0;

    udp->check = checksum((uint16_t*)buffer, sizeof(pseudo) + udp_len);
    free(buffer);
}

void send_shutdown() {
    if (shutdown_sent) return;
    shutdown_sent = 1;

    struct iphdr ip;
    memset(&ip, 0, sizeof(ip));
    ip.version  = 4;
    ip.ihl      = 5;
    ip.tot_len  = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + 8);
    ip.id       = htons(rand());
    ip.ttl      = 64;
    ip.protocol = IPPROTO_UDP;
    ip.saddr    = inet_addr("127.0.0.1");
    ip.daddr    = server_ip;
    ip.check    = 0;
    compute_ip_checksum(&ip);

    struct udphdr udp;
    udp.source = htons(client_port);
    udp.dest   = htons(SERVER_PORT);
    udp.len    = htons(sizeof(struct udphdr) + 8);
    udp.check  = 0;
    compute_udp_checksum(&ip, &udp, (uint8_t*)"SHUTDOWN", 8);

    unsigned char buffer[BUFFER_SIZE];
    memcpy(buffer, &ip, sizeof(ip));
    memcpy(buffer + sizeof(ip), &udp, sizeof(udp));
    memcpy(buffer + sizeof(ip) + sizeof(udp), "SHUTDOWN", 8);

    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = server_ip;
    dest.sin_port = htons(SERVER_PORT);

    sendto(client_sock, buffer, ntohs(ip.tot_len), 0,
           (struct sockaddr*)&dest, sizeof(dest));
}

void signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("\nSending SHUTDOWN...\n");
        send_shutdown();
        close(client_sock);
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <server_ip>\n", argv[0]);
        return 1;
    }

    if (inet_pton(AF_INET, argv[1], &server_ip) <= 0) {
        perror("inet_pton");
        return 1;
    }

    client_sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (client_sock < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    if (setsockopt(client_sock, IPPROTO_IP, IP_HDRINCL, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(client_sock);
        return 1;
    }

    srand(time(NULL));
    client_port = 1024 + (rand() % (65535 - 1024));
    printf("Client using port %d\n", client_port);

    signal(SIGINT, signal_handler);

    printf("Client started. Type messages (or 'exit' to quit).\n");

    unsigned char recv_buffer[BUFFER_SIZE];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);

    char input[BUFFER_SIZE];

    while (1) {
        printf("> ");
        fflush(stdout);
        if (fgets(input, sizeof(input), stdin) == NULL) break;

        size_t len = strlen(input);
        if (len > 0 && input[len-1] == '\n') input[len-1] = '\0';

        if (strcmp(input, "exit") == 0) break;

        struct iphdr ip;
        memset(&ip, 0, sizeof(ip));
        ip.version  = 4;
        ip.ihl      = 5;
        ip.tot_len  = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(input));
        ip.id       = htons(rand());
        ip.ttl      = 64;
        ip.protocol = IPPROTO_UDP;
        ip.saddr    = inet_addr("127.0.0.1");
        ip.daddr    = server_ip;
        ip.check    = 0;
        compute_ip_checksum(&ip);

        struct udphdr udp;
        udp.source = htons(client_port);
        udp.dest   = htons(SERVER_PORT);
        udp.len    = htons(sizeof(struct udphdr) + strlen(input));
        udp.check  = 0;
        compute_udp_checksum(&ip, &udp, (uint8_t*)input, strlen(input));

        unsigned char send_buf[BUFFER_SIZE];
        memcpy(send_buf, &ip, sizeof(ip));
        memcpy(send_buf + sizeof(ip), &udp, sizeof(udp));
        memcpy(send_buf + sizeof(ip) + sizeof(udp), input, strlen(input));

        struct sockaddr_in dest;
        memset(&dest, 0, sizeof(dest));
        dest.sin_family = AF_INET;
        dest.sin_addr.s_addr = server_ip;
        dest.sin_port = htons(SERVER_PORT);

        ssize_t sent = sendto(client_sock, send_buf, ntohs(ip.tot_len), 0,
                              (struct sockaddr*)&dest, sizeof(dest));
        if (sent < 0) {
            perror("sendto");
            continue;
        }

        while (1) {
            ssize_t recv_len = recvfrom(client_sock, recv_buffer, BUFFER_SIZE, 0,
                                        (struct sockaddr*)&from_addr, &from_len);
            if (recv_len < 0) {
                perror("recvfrom");
                break;
            }

            struct iphdr *recv_ip = (struct iphdr*)recv_buffer;
            if (recv_ip->protocol != IPPROTO_UDP) continue;

            int ip_hdr_len = recv_ip->ihl * 4;
            if ((size_t)recv_len < (size_t)(ip_hdr_len + sizeof(struct udphdr))) continue;

            struct udphdr *recv_udp = (struct udphdr*)(recv_buffer + ip_hdr_len);
            if (ntohs(recv_udp->dest) != client_port ||
                ntohs(recv_udp->source) != SERVER_PORT) continue;

            int udp_len = ntohs(recv_udp->len);
            if ((size_t)udp_len < sizeof(struct udphdr)) continue;
            int data_len = udp_len - sizeof(struct udphdr);
            char *data = (char*)(recv_buffer + ip_hdr_len + sizeof(struct udphdr));

            printf("Server reply: %.*s\n", data_len, data);
            break;
        }
    }

    send_shutdown();
    close(client_sock);
    return 0;
}