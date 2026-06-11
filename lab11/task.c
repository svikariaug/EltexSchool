#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_STALLS 5
#define INIT_MIN 900
#define INIT_MAX 1100
#define NEED_MIN 9900
#define NEED_MAX 10100
#define LOADER_ADD 200
#define LOADER_SLEEP 1
#define BUYER_SLEEP 2

typedef struct {
    int goods;
    pthread_mutex_t mutex;
} Stall;

Stall stalls[NUM_STALLS];

int buyers_finished = 0;
pthread_mutex_t finished_mutex = PTHREAD_MUTEX_INITIALIZER;

int random_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

void log_message(const char* thread_type, int thread_id, const char* message) {
    time_t now;
    time(&now);
    struct tm* tm_info = localtime(&now);
    char time_buffer[20];
    strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", tm_info);
    printf("[%s] %s %d: %s\n", time_buffer, thread_type, thread_id, message);
    fflush(stdout);
}

void* buyer(void* arg) {
    int id = *(int*)arg;
    free(arg);
    
    int need = random_range(NEED_MIN, NEED_MAX);
    char log_buf[256];
    
    sprintf(log_buf, "проснулся, текущее кол-во потребности = %d", need);
    log_message("Покупатель", id, log_buf);
    
    while (need > 0) {
        int stall_index = -1;
        
        for (int i = 0; i < NUM_STALLS; i++) {
            if (pthread_mutex_trylock(&stalls[i].mutex) == 0) {
                stall_index = i;
                break;
            }
        }
        
        if (stall_index == -1) {
            usleep(100000);
            continue;
        }
        
        sprintf(log_buf, "зашел в ларек %d, количество товаров %d", 
                stall_index, stalls[stall_index].goods);
        log_message("Покупатель", id, log_buf);
        
        while (need > 0 && stalls[stall_index].goods > 0) {
            stalls[stall_index].goods--;
            need--;
            
            if (stalls[stall_index].goods == 0 || need == 0) {
                sprintf(log_buf, "стало потребности %d", need);
                log_message("Покупатель", id, log_buf);
            }
        }
        
        sprintf(log_buf, "вышел из ларька %d", stall_index);
        log_message("Покупатель", id, log_buf);
        
        pthread_mutex_unlock(&stalls[stall_index].mutex);
        
        if (need > 0) {
            sprintf(log_buf, "уснул на %d секунд", BUYER_SLEEP);
            log_message("Покупатель", id, log_buf);
            sleep(BUYER_SLEEP);
            sprintf(log_buf, "проснулся, текущее кол-во потребности = %d", need);
            log_message("Покупатель", id, log_buf);
        }
    }
    
    sprintf(log_buf, "утолил потребность, уходит");
    log_message("Покупатель", id, log_buf);
    
    pthread_mutex_lock(&finished_mutex);
    buyers_finished++;
    pthread_mutex_unlock(&finished_mutex);
    
    return NULL;
}

void* loader(void* arg) {
    int id = *(int*)arg;
    free(arg);
    
    while (1) {
        pthread_mutex_lock(&finished_mutex);
        if (buyers_finished >= 3) {
            pthread_mutex_unlock(&finished_mutex);
            break;
        }
        pthread_mutex_unlock(&finished_mutex);
        
        int stall_index = rand() % NUM_STALLS;
        
        pthread_mutex_lock(&stalls[stall_index].mutex);
        
        char log_buf[256];
        sprintf(log_buf, "зашел в ларек %d, текущий запас %d", 
                stall_index, stalls[stall_index].goods);
        log_message("Погрузчик", id, log_buf);
        
        stalls[stall_index].goods += LOADER_ADD;
        
        sprintf(log_buf, "пополнил на %d, новый запас %d", 
                LOADER_ADD, stalls[stall_index].goods);
        log_message("Погрузчик", id, log_buf);
        
        pthread_mutex_unlock(&stalls[stall_index].mutex);
        
        sprintf(log_buf, "уснул на %d секунд", LOADER_SLEEP);
        log_message("Погрузчик", id, log_buf);
        sleep(LOADER_SLEEP);
    }
    
    log_message("Погрузчик", id, "завершается");
    return NULL;
}

int main() {
    srand(time(NULL));
    
    for (int i = 0; i < NUM_STALLS; i++) {
        stalls[i].goods = random_range(INIT_MIN, INIT_MAX);
        pthread_mutex_init(&stalls[i].mutex, NULL);
        printf("[INIT] Ларек %d: начальный запас %d\n", i, stalls[i].goods);
    }
    
    pthread_t buyers[3];
    pthread_t loader_thread;
    
    for (int i = 0; i < 3; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&buyers[i], NULL, buyer, id);
    }
    
    int* loader_id = malloc(sizeof(int));
    *loader_id = 1;
    pthread_create(&loader_thread, NULL, loader, loader_id);
    
    for (int i = 0; i < 3; i++) {
        pthread_join(buyers[i], NULL);
    }
    
    pthread_join(loader_thread, NULL);
    
    for (int i = 0; i < NUM_STALLS; i++) {
        pthread_mutex_destroy(&stalls[i].mutex);
    }
    pthread_mutex_destroy(&finished_mutex);
    
    printf("[MAIN] Все покупатели насытились, программа завершена.\n");
    return 0;
}