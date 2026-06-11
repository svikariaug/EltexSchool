#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/socket.h>

#define PORT 8084
#define BUFFER_SIZE 256
#define THREAD_POOL_SIZE 5
#define QUEUE_SIZE 10

typedef struct {
    int client_fd;
    struct sockaddr_in client_addr;
} request_t;

request_t queue[QUEUE_SIZE];
int front = 0, rear = 0, count = 0;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_not_full = PTHREAD_COND_INITIALIZER;

void* worker_function(void* arg) {
    int id = *(int*)arg;
    request_t request;
    char buffer[BUFFER_SIZE];
    time_t now;
    
    while (1) {
        pthread_mutex_lock(&queue_mutex);
        while (count == 0) {
            pthread_cond_wait(&queue_not_empty, &queue_mutex);
        }
        
        request = queue[front];
        front = (front + 1) % QUEUE_SIZE;
        count--;
        
        pthread_cond_signal(&queue_not_full);
        pthread_mutex_unlock(&queue_mutex);
        
        printf("Worker %d processing client\n", id);
        
        memset(buffer, 0, BUFFER_SIZE);
        read(request.client_fd, buffer, BUFFER_SIZE);
        
        now = time(NULL);
        strcpy(buffer, ctime(&now));
        write(request.client_fd, buffer, strlen(buffer) + 1);
        
        close(request.client_fd);
    }
    
    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    pthread_t threads[THREAD_POOL_SIZE];
    int ids[THREAD_POOL_SIZE];
    request_t new_request;
    
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
    printf("Time Server (Queue) on port %d\n", PORT);
    
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, worker_function, &ids[i]);
    }
    
    while (1) {
        client_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        
        new_request.client_fd = client_fd;
        new_request.client_addr = client_addr;
        
        pthread_mutex_lock(&queue_mutex);
        while (count == QUEUE_SIZE) {
            pthread_cond_wait(&queue_not_full, &queue_mutex);
        }
        
        queue[rear] = new_request;
        rear = (rear + 1) % QUEUE_SIZE;
        count++;
        
        pthread_cond_signal(&queue_not_empty);
        pthread_mutex_unlock(&queue_mutex);
    }
    
    close(server_fd);
    return 0;
}