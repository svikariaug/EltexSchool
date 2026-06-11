#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>

#define SHM_NAME "/myshm"
#define SEM_NAME "/mysem"
#define MSG_SIZE 256

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    char *shm_ptr = mmap(NULL, MSG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    sem_t *sem = sem_open(SEM_NAME, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    printf("[Client] Received: %s\n", shm_ptr);

    strcpy(shm_ptr, "Hello!");
    printf("[Client] Sent: %s\n", shm_ptr);

    sem_post(sem);

    munmap(shm_ptr, MSG_SIZE);
    close(shm_fd);
    sem_close(sem);
    return 0;
}