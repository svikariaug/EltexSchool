#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>

#define SERVER_QUEUE "/server_queue"
#define CLIENT_QUEUE "/client_queue"
#define MAX_MSG_SIZE 256

int main() {
    mqd_t mq_server, mq_client;
    char buffer[MAX_MSG_SIZE];

    // Удаляем старые очереди, если есть
    mq_unlink(SERVER_QUEUE);
    mq_unlink(CLIENT_QUEUE);

    // Создаём очередь сервера
    struct mq_attr attr = {0, 10, MAX_MSG_SIZE, 0};
    mq_server = mq_open(SERVER_QUEUE, O_CREAT | O_RDWR, 0666, &attr);
    if (mq_server == -1) {
        perror("mq_open server");
        exit(1);
    }

    // Создаём очередь клиента
    mq_client = mq_open(CLIENT_QUEUE, O_CREAT | O_RDWR, 0666, &attr);
    if (mq_client == -1) {
        perror("mq_open client");
        exit(1);
    }

    // Отправляем "Hi!"
    strcpy(buffer, "Hi!");
    if (mq_send(mq_server, buffer, strlen(buffer) + 1, 1) == -1) {
        perror("mq_send");
        exit(1);
    }
    printf("Server sent: %s\n", buffer);

    // Ждём ответ от клиента
    if (mq_receive(mq_client, buffer, MAX_MSG_SIZE, NULL) == -1) {
        perror("mq_receive");
        exit(1);
    }
    printf("Server received: %s\n", buffer);

    // Удаляем очереди
    mq_close(mq_server);
    mq_close(mq_client);
    mq_unlink(SERVER_QUEUE);
    mq_unlink(CLIENT_QUEUE);

    return 0;
}