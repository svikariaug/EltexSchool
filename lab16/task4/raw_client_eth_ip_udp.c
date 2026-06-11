#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <sys/ioctl.h>

#define PORT 8086
#define BUFFER_SIZE 2048
#define INTERFACE "lo"

struct pseudo_header {
    uint32_t src_addr;
    uint32_t dst_addr;
    uint8_t zero;
    uint8_t protocol;
    uint16_t udp_len;
};

uint16_t calculate_checksum(uint16_t *addr, int len) {
    int nleft = len;
    int sum = 0;
    uint16_t *w = addr;
    uint16_t answer = 0;
    
    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }
    
    if (nleft == 1) {
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }
    
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    answer = ~sum;
    return answer;
}

void get_mac_address(int sock, const char* iface, unsigned char* mac) {
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, iface);
    
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
        perror("ioctl");
        exit(1);
    }
    
    memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
}

int main() {
    int raw_fd;
    struct sockaddr_ll dest_addr;
    char packet[BUFFER_SIZE];
    struct ethhdr *eth_header;
    struct iphdr *ip_header;
    struct udphdr *udp_header;
    char *data;
    int packet_size;
    unsigned char src_mac[6], dst_mac[6];
    struct pseudo_header pseudo;
    char recv_buffer[BUFFER_SIZE];
    
    printf("=== RAW SOCKET: ЗАДАНИЕ 3 ===\n");
    printf("Клиент формирует Ethernet + IP + UDP заголовки + данные\n\n");
    
    raw_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_fd < 0) {
        perror("socket");
        exit(1);
    }
    
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sll_family = AF_PACKET;
    dest_addr.sll_ifindex = if_nametoindex(INTERFACE);
    dest_addr.sll_protocol = htons(ETH_P_IP);
    
    memset(dst_mac, 0xFF, 6);
    get_mac_address(raw_fd, INTERFACE, src_mac);
    
    memset(packet, 0, BUFFER_SIZE);
    
    eth_header = (struct ethhdr *)packet;
    ip_header = (struct iphdr *)(packet + sizeof(struct ethhdr));
    udp_header = (struct udphdr *)(packet + sizeof(struct ethhdr) + sizeof(struct iphdr));
    data = packet + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr);
    
    strcpy(data, "Hello from Task 3: Ethernet+IP+UDP headers!");
    
    udp_header->source = htons(12345);
    udp_header->dest = htons(PORT);
    udp_header->len = htons(sizeof(struct udphdr) + strlen(data) + 1);
    udp_header->check = 0;
    
    ip_header->ihl = 5;
    ip_header->version = 4;
    ip_header->tos = 0;
    ip_header->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data) + 1);
    ip_header->id = htons(54321);
    ip_header->frag_off = 0;
    ip_header->ttl = 64;
    ip_header->protocol = IPPROTO_UDP;
    ip_header->check = 0;
    ip_header->saddr = inet_addr("127.0.0.1");
    ip_header->daddr = inet_addr("127.0.0.1");
    
    ip_header->check = calculate_checksum((uint16_t *)ip_header, sizeof(struct iphdr));
    
    pseudo.src_addr = ip_header->saddr;
    pseudo.dst_addr = ip_header->daddr;
    pseudo.zero = 0;
    pseudo.protocol = IPPROTO_UDP;
    pseudo.udp_len = udp_header->len;
    
    int pseudo_size = sizeof(struct pseudo_header) + ntohs(udp_header->len);
    char *pseudo_buffer = malloc(pseudo_size);
    memcpy(pseudo_buffer, &pseudo, sizeof(struct pseudo_header));
    memcpy(pseudo_buffer + sizeof(struct pseudo_header), udp_header, ntohs(udp_header->len));
    
    udp_header->check = calculate_checksum((uint16_t *)pseudo_buffer, pseudo_size);
    free(pseudo_buffer);
    
    memcpy(eth_header->h_dest, dst_mac, 6);
    memcpy(eth_header->h_source, src_mac, 6);
    eth_header->h_proto = htons(ETH_P_IP);
    
    packet_size = sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data) + 1;
    
    printf("Сформирован Ethernet+IP+UDP пакет:\n");
    printf("  - Данные: %s\n", data);
    printf("  - Размер: %d байт\n\n", packet_size);
    
    if (sendto(raw_fd, packet, packet_size, 0,
               (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("sendto");
        exit(1);
    }
    
    printf("Пакет отправлен. Ожидание ответа...\n");
    
    struct sockaddr_ll recv_addr;
    socklen_t addr_len = sizeof(recv_addr);
    
    while (1) {
        memset(recv_buffer, 0, BUFFER_SIZE);
        
        if (recvfrom(raw_fd, recv_buffer, BUFFER_SIZE, 0,
                     (struct sockaddr*)&recv_addr, &addr_len) < 0) {
            perror("recvfrom");
            continue;
        }
        
        struct ethhdr *resp_eth = (struct ethhdr *)recv_buffer;
        
        if (ntohs(resp_eth->h_proto) == ETH_P_IP) {
            struct iphdr *resp_ip = (struct iphdr *)(recv_buffer + sizeof(struct ethhdr));
            
            if (resp_ip->protocol == IPPROTO_UDP) {
                struct udphdr *resp_udp = (struct udphdr *)(recv_buffer + sizeof(struct ethhdr) + (resp_ip->ihl * 4));
                
                if (ntohs(resp_udp->source) == PORT) {
                    char *resp_data = recv_buffer + sizeof(struct ethhdr) + (resp_ip->ihl * 4) + sizeof(struct udphdr);
                    printf("\nОтвет от сервера: %s\n", resp_data);
                    break;
                }
            }
        }
    }
    
    close(raw_fd);
    return 0;
}