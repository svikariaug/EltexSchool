#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>

#define SHM_NAME "/chat_shm"
#define SEM_MUTEX "/chat_mutex"
#define SEM_MSG_COUNT "/chat_msg_count"
#define MAX_CLIENTS 20
#define MAX_MSG_SIZE 512
#define MAX_NAME_LEN 32
#define MAX_MESSAGES 100

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    int active;
    time_t join_time;
} Client;

typedef struct {
    int id;
    int timestamp;
    char sender_name[MAX_NAME_LEN];
    char content[MAX_MSG_SIZE];
} Message;

typedef struct {
    int client_count;
    int next_id;
    int msg_count;
    int msg_head;
    int msg_tail;
    Client clients[MAX_CLIENTS];
    Message messages[MAX_MESSAGES];
    char user_list[4096];
} ChatShared;

static ChatShared *shared = NULL;
static int shm_fd;
static sem_t *mutex;
static sem_t *msg_count;
static volatile sig_atomic_t running = 1;

static void cleanup(int signum) {
    (void)signum;
    running = 0;
    
    if (shared) {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (shared->clients[i].active) {
                shared->clients[i].active = 0;
            }
        }
        munmap(shared, sizeof(ChatShared));
    }
    
    if (shm_fd > 0) {
        close(shm_fd);
        shm_unlink(SHM_NAME);
    }
    
    if (mutex != NULL && mutex != SEM_FAILED) {
        sem_close(mutex);
        sem_unlink(SEM_MUTEX);
    }
    
    if (msg_count != NULL && msg_count != SEM_FAILED) {
        sem_close(msg_count);
        sem_unlink(SEM_MSG_COUNT);
    }
    
    printf("\nServer stopped\n");
    exit(0);
}

static void update_user_list(void) {
    char temp[4096] = "";
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (shared->clients[i].active) {
            if (strlen(temp) > 0) {
                strcat(temp, ",");
            }
            strcat(temp, shared->clients[i].name);
        }
    }
    
    strcpy(shared->user_list, temp);
}

static void add_system_message(const char *content) {
    int next = (shared->msg_tail + 1) % MAX_MESSAGES;
    
    if (next != shared->msg_head) {
        shared->messages[shared->msg_tail].id = -1;
        strcpy(shared->messages[shared->msg_tail].sender_name, "SYSTEM");
        strcpy(shared->messages[shared->msg_tail].content, content);
        shared->messages[shared->msg_tail].timestamp = (int)time(NULL);
        shared->msg_tail = next;
        shared->msg_count++;
        sem_post(msg_count);
    }
}

static int register_client(const char *name, int *assigned_id) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (shared->clients[i].active && strcmp(shared->clients[i].name, name) == 0) {
            return -1;
        }
    }
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!shared->clients[i].active) {
            *assigned_id = shared->next_id++;
            strcpy(shared->clients[i].name, name);
            shared->clients[i].id = *assigned_id;
            shared->clients[i].active = 1;
            shared->clients[i].join_time = time(NULL);
            shared->client_count++;
            return i;
        }
    }
    
    return -1;
}

static void* message_dispatcher(void *arg) {
    (void)arg;
    
    while (running) {
        sem_wait(msg_count);
        if (!running) break;
    }
    
    return NULL;
}

int main(void) {
    signal(SIGINT, cleanup);
    
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_MUTEX);
    sem_unlink(SEM_MSG_COUNT);
    
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    
    if (ftruncate(shm_fd, sizeof(ChatShared)) == -1) {
        perror("ftruncate");
        exit(1);
    }
    
    shared = mmap(0, sizeof(ChatShared), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    
    mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    msg_count = sem_open(SEM_MSG_COUNT, O_CREAT, 0666, 0);
    
    if (mutex == SEM_FAILED || msg_count == SEM_FAILED) {
        perror("sem_open");
        cleanup(0);
        exit(1);
    }
    
    sem_wait(mutex);
    memset(shared, 0, sizeof(ChatShared));
    shared->client_count = 0;
    shared->next_id = 1;
    shared->msg_count = 0;
    shared->msg_head = 0;
    shared->msg_tail = 0;
    strcpy(shared->user_list, "");
    sem_post(mutex);
    
    pthread_t dispatcher_thread;
    pthread_create(&dispatcher_thread, NULL, message_dispatcher, NULL);
    
    printf("Chat server started\n");
    printf("Shared memory: %s\n", SHM_NAME);
    printf("Max clients: %d\n", MAX_CLIENTS);
    printf("Press Ctrl+C to stop\n\n");
    
    while (running) {
        sleep(1);
    }
    
    pthread_join(dispatcher_thread, NULL);
    cleanup(0);
    
    return 0;
}