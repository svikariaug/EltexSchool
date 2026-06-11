#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/socket.h>

#define PORT 8083
#define BUFFER_SIZE 256
#define THREAD_POOL_SIZE 5

typedef struct {
    int client_fd;
    int is_occupied;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} worker_t;

worker_t workers[THREAD_POOL_SIZE];
pthread_mutex_t server_mutex = PTHREAD_MUTEX_INITIALIZER;

void* worker_function(void* arg) {
    int id = *(int*)arg;
    worker_t* worker = &workers[id];
    char buffer[BUFFER_SIZE];
    time_t now;
    
    while (1) {
        pthread_mutex_lock(&worker->mutex);
        while (!worker->is_occupied) {
            pthread_cond_wait(&worker->cond, &worker->mutex);
        }
        
        printf("Worker %d processing client\n", id);
        
        memset(buffer, 0, BUFFER_SIZE);
        read(worker->client_fd, buffer, BUFFER_SIZE);
        
        now = time(NULL);
        strcpy(buffer, ctime(&now));
        write(worker->client_fd, buffer, strlen(buffer) + 1);
        
        close(worker->client_fd);
        worker->is_occupied = 0;
        
        pthread_mutex_unlock(&worker->mutex);
    }
    
    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr;
    pthread_t threads[THREAD_POOL_SIZE];
    int ids[THREAD_POOL_SIZE];
    
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
    printf("Time Server (Thread Pool) on port %d\n", PORT);
    
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        workers[i].is_occupied = 0;
        pthread_mutex_init(&workers[i].mutex, NULL);
        pthread_cond_init(&workers[i].cond, NULL);
        ids[i] = i;
        pthread_create(&threads[i], NULL, worker_function, &ids[i]);
    }
    
    while (1) {
        client_fd = accept(server_fd, NULL, NULL);
        
        pthread_mutex_lock(&server_mutex);
        for (int i = 0; i < THREAD_POOL_SIZE; i++) {
            pthread_mutex_lock(&workers[i].mutex);
            if (!workers[i].is_occupied) {
                workers[i].client_fd = client_fd;
                workers[i].is_occupied = 1;
                pthread_cond_signal(&workers[i].cond);
                pthread_mutex_unlock(&workers[i].mutex);
                break;
            }
            pthread_mutex_unlock(&workers[i].mutex);
        }
        pthread_mutex_unlock(&server_mutex);
    }
    
    close(server_fd);
    return 0;
}