#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define MSG_KEY_SERVER 1234
#define MSG_KEY_CLIENT 5678

struct msgbuf {
    long mtype;
    char mtext[256];
};

int main() {
    int msgid_server, msgid_client;
    struct msgbuf buf;

    // Создаём очереди
    msgid_server = msgget(MSG_KEY_SERVER, IPC_CREAT | 0666);
    if (msgid_server == -1) {
        perror("msgget server");
        exit(1);
    }

    msgid_client = msgget(MSG_KEY_CLIENT, IPC_CREAT | 0666);
    if (msgid_client == -1) {
        perror("msgget client");
        exit(1);
    }

    // Отправляем "Hi!"
    buf.mtype = 1;
    strcpy(buf.mtext, "Hi!");
    if (msgsnd(msgid_server, &buf, strlen(buf.mtext) + 1, 0) == -1) {
        perror("msgsnd");
        exit(1);
    }
    printf("Server sent: %s\n", buf.mtext);

    // Ждём ответ
    if (msgrcv(msgid_client, &buf, sizeof(buf.mtext), 1, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }
    printf("Server received: %s\n", buf.mtext);

    // Удаляем очереди
    msgctl(msgid_server, IPC_RMID, NULL);
    msgctl(msgid_client, IPC_RMID, NULL);

    return 0;
}