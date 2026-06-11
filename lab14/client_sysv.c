#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY 1234
#define MSG_SIZE 256

int main() {
    int shmid = shmget(SHM_KEY, MSG_SIZE, 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    char *shm_ptr = shmat(shmid, NULL, 0);
    if (shm_ptr == (char *)-1) {
        perror("shmat");
        exit(1);
    }

    printf("[Client] Received: %s\n", shm_ptr);
    strcpy(shm_ptr, "Hello!");
    printf("[Client] Sent: %s\n", shm_ptr);

    shmdt(shm_ptr);
    return 0;
}