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

    mq_server = mq_open(SERVER_QUEUE, O_RDWR);
    if (mq_server == -1) {
        perror("mq_open server");
        exit(1);
    }

    mq_client = mq_open(CLIENT_QUEUE, O_RDWR);
    if (mq_client == -1) {
        perror("mq_open client");
        exit(1);
    }

    if (mq_receive(mq_server, buffer, MAX_MSG_SIZE, NULL) == -1) {
        perror("mq_receive");
        exit(1);
    }
    printf("Client received: %s\n", buffer);

    strcpy(buffer, "Hello!");
    if (mq_send(mq_client, buffer, strlen(buffer) + 1, 1) == -1) {
        perror("mq_send");
        exit(1);
    }
    printf("Client sent: %s\n", buffer);

    mq_close(mq_server);
    mq_close(mq_client);

    return 0;
}