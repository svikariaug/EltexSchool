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

#define SERVER_PORT     8888
#define BUFFER_SIZE     65536
#define MAX_CLIENTS     100

struct client_entry {
    uint32_t ip;
    uint16_t port;
    int counter;
};

struct client_entry clients[MAX_CLIENTS];
int num_clients = 0;
int server_sock;

void remove_client(uint32_t ip, uint16_t port) {
    for (int i = 0; i < num_clients; i++) {
        if (clients[i].ip == ip && clients[i].port == port) {
            for (int j = i; j < num_clients - 1; j++)
                clients[j] = clients[j + 1];
            num_clients--;
            break;
        }
    }
}

struct client_entry* find_client(uint32_t ip, uint16_t port) {
    for (int i = 0; i < num_clients; i++)
        if (clients[i].ip == ip && clients[i].port == port)
            return &clients[i];
    return NULL;
}

struct client_entry* add_client(uint32_t ip, uint16_t port) {
    if (num_clients >= MAX_CLIENTS) return NULL;
    clients[num_clients].ip = ip;
    clients[num_clients].port = port;
    clients[num_clients].counter = 0;
    num_clients++;
    return &clients[num_clients - 1];
}

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

void signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("\nServer shutting down...\n");
        close(server_sock);
        exit(0);
    }
}

int main() {
    signal(SIGINT, signal_handler);

    server_sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (server_sock < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_sock, IPPROTO_IP, IP_HDRINCL, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_sock);
        return 1;
    }

    printf("Echo server started on port %d (raw socket).\n", SERVER_PORT);

    unsigned char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while (1) {
        ssize_t recv_len = recvfrom(server_sock, buffer, BUFFER_SIZE, 0,
                                    (struct sockaddr*)&client_addr, &addr_len);
        if (recv_len < 0) {
            perror("recvfrom");
            continue;
        }

        struct iphdr *ip = (struct iphdr*)buffer;
        if (ip->protocol != IPPROTO_UDP) continue;

        int ip_hdr_len = ip->ihl * 4;
        if ((size_t)recv_len < (size_t)(ip_hdr_len + sizeof(struct udphdr))) continue;

        struct udphdr *udp = (struct udphdr*)(buffer + ip_hdr_len);
        if (ntohs(udp->dest) != SERVER_PORT) continue;

        int udp_len = ntohs(udp->len);
        if ((size_t)udp_len < sizeof(struct udphdr)) continue;
        int data_len = udp_len - sizeof(struct udphdr);
        uint8_t *data = buffer + ip_hdr_len + sizeof(struct udphdr);

        uint32_t client_ip = ip->saddr;
        uint16_t client_port = ntohs(udp->source);

        struct client_entry *entry = find_client(client_ip, client_port);
        if (!entry) {
            entry = add_client(client_ip, client_port);
            if (!entry) {
                fprintf(stderr, "Too many clients, dropping packet\n");
                continue;
            }
        }

        if (data_len == 8 && strncmp((char*)data, "SHUTDOWN", 8) == 0) {
            printf("Client %s:%d sent SHUTDOWN, removing.\n",
                   inet_ntoa(*(struct in_addr*)&client_ip), client_port);
            remove_client(client_ip, client_port);
            continue;
        }

        entry->counter++;
        int counter = entry->counter;

        char response[BUFFER_SIZE];
        int resp_len = snprintf(response, sizeof(response), "%.*s %d",
                                data_len, (char*)data, counter);
        if (resp_len >= BUFFER_SIZE) resp_len = BUFFER_SIZE - 1;

        printf("Received from %s:%d: %.*s (counter %d)\n",
               inet_ntoa(*(struct in_addr*)&client_ip), client_port,
               data_len, (char*)data, counter);

        struct udphdr resp_udp;
        resp_udp.source = htons(SERVER_PORT);
        resp_udp.dest   = htons(client_port);
        resp_udp.len    = htons(sizeof(struct udphdr) + resp_len);
        resp_udp.check  = 0;

        struct iphdr resp_ip;
        memset(&resp_ip, 0, sizeof(resp_ip));
        resp_ip.version  = 4;
        resp_ip.ihl      = 5;
        resp_ip.tot_len  = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + resp_len);
        resp_ip.id       = htons(rand());
        resp_ip.ttl      = 64;
        resp_ip.protocol = IPPROTO_UDP;
        resp_ip.saddr    = ip->daddr;
        resp_ip.daddr    = ip->saddr;
        resp_ip.check    = 0;

        compute_ip_checksum(&resp_ip);
        compute_udp_checksum(&resp_ip, &resp_udp, (uint8_t*)response, resp_len);

        unsigned char send_buf[BUFFER_SIZE];
        memcpy(send_buf, &resp_ip, sizeof(resp_ip));
        memcpy(send_buf + sizeof(resp_ip), &resp_udp, sizeof(resp_udp));
        memcpy(send_buf + sizeof(resp_ip) + sizeof(resp_udp), response, resp_len);

        struct sockaddr_in dest_addr;
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_addr.s_addr = ip->saddr;
        dest_addr.sin_port = htons(client_port);

        ssize_t sent = sendto(server_sock, send_buf, ntohs(resp_ip.tot_len), 0,
                              (struct sockaddr*)&dest_addr, sizeof(dest_addr));
        if (sent < 0) perror("sendto");
    }

    close(server_sock);
    return 0;
}