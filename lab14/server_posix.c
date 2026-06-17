#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>

#define SHM_NAME "/myshm"
#define SEM_NAME "/mysem"
#define MSG_SIZE 256

int main() {
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    if (ftruncate(shm_fd, MSG_SIZE) == -1) {
        perror("ftruncate");
        exit(1);
    }

    char *shm_ptr = mmap(NULL, MSG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    strcpy(shm_ptr, "Hi!");
    printf("[Server] Sent: %s\n", shm_ptr);

    sem_wait(sem);
    printf("[Server] Received: %s\n", shm_ptr);

    munmap(shm_ptr, MSG_SIZE);
    close(shm_fd);
    shm_unlink(SHM_NAME);
    sem_close(sem);
    sem_unlink(SEM_NAME);
    return 0;
}