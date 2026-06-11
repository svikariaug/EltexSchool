#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MSG_KEY_SERVER 1234
#define MSG_KEY_CLIENT 5678

struct msgbuf {
    long mtype;
    char mtext[256];
};

int main() {
    int msgid_server, msgid_client;
    struct msgbuf buf;

    msgid_server = msgget(MSG_KEY_SERVER, 0666);
    if (msgid_server == -1) {
        perror("msgget server");
        exit(1);
    }

    msgid_client = msgget(MSG_KEY_CLIENT, 0666);
    if (msgid_client == -1) {
        perror("msgget client");
        exit(1);
    }

    if (msgrcv(msgid_server, &buf, sizeof(buf.mtext), 1, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }
    printf("Client received: %s\n", buf.mtext);

    buf.mtype = 1;
    strcpy(buf.mtext, "Hello!");
    if (msgsnd(msgid_client, &buf, strlen(buf.mtext) + 1, 0) == -1) {
        perror("msgsnd");
        exit(1);
    }
    printf("Client sent: %s\n", buf.mtext);

    return 0;
}   