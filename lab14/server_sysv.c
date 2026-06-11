#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_KEY 1234
#define MSG_SIZE 256

int main() {
    // Создаём сегмент
    int shmid = shmget(SHM_KEY, MSG_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    char *shm_ptr = shmat(shmid, NULL, 0);
    if (shm_ptr == (char *)-1) {
        perror("shmat");
        exit(1);
    }

    // Записываем "Hi!"
    strcpy(shm_ptr, "Hi!");
    printf("[Server] Sent: %s\n", shm_ptr);

    // Ждём, пока клиент изменит данные (простая задержка)
    while (strcmp(shm_ptr, "Hi!") == 0) {
        sleep(1);
    }
    printf("[Server] Received: %s\n", shm_ptr);

    // Отключаем и удаляем
    shmdt(shm_ptr);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}