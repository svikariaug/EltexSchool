#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define SERVER_QUEUE "/chat_server"
#define MAX_CLIENTS 10
#define MAX_MSG_SIZE 512
#define MAX_NAME_LEN 32
#define MAX_QUEUE_NAME 64

typedef struct {
    char name[MAX_NAME_LEN];
    char queue_name[MAX_QUEUE_NAME];
    mqd_t queue;
    int active;
    int id;
} Client;

Client clients[MAX_CLIENTS];
mqd_t server_q;
int running = 1;
int next_id = 0;

void cleanup() {
    running = 0;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            char close_msg[MAX_MSG_SIZE];
            strcpy(close_msg, "[SERVER] Server shutting down.");
            mq_send(clients[i].queue, close_msg, strlen(close_msg) + 1, 1);
            mq_close(clients[i].queue);
            mq_unlink(clients[i].queue_name);
        }
    }

    if (server_q > 0) {
        mq_close(server_q);
        mq_unlink(SERVER_QUEUE);
    }
}

void send_to_all(char *msg, int sender_id, int include_sender) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            if (include_sender || clients[i].id != sender_id) {
                mq_send(clients[i].queue, msg, strlen(msg) + 1, 1);
            }
        }
    }
}

void broadcast_user_list() {
    char list_msg[MAX_MSG_SIZE];
    strcpy(list_msg, "[SERVER] Users in chat: ");

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            strcat(list_msg, clients[i].name);
            strcat(list_msg, " ");
        }
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            mq_send(clients[i].queue, list_msg, strlen(list_msg) + 1, 1);
        }
    }
}

int find_free_slot() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active) return i;
    }
    return -1;
}

int find_client_by_id(int id) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && clients[i].id == id) return i;
    }
    return -1;
}

void* handle_clients(void *arg) {
    char buffer[MAX_MSG_SIZE];
    unsigned int prio;

    (void)arg;

    while (running) {
        if (mq_receive(server_q, buffer, MAX_MSG_SIZE, &prio) > 0) {
            char cmd[10], data[MAX_MSG_SIZE];
            sscanf(buffer, "%[^|]|%[^\n]", cmd, data);

            if (strcmp(cmd, "REG") == 0) {
                char name[MAX_NAME_LEN], queue_name[MAX_QUEUE_NAME];
                sscanf(data, "%[^|]|%[^\n]", name, queue_name);

                int slot = find_free_slot();
                if (slot >= 0) {
                    strcpy(clients[slot].name, name);
                    strcpy(clients[slot].queue_name, queue_name);
                    clients[slot].active = 1;
                    clients[slot].id = next_id++;

                    clients[slot].queue = mq_open(queue_name, O_RDWR);
                    if (clients[slot].queue == -1) {
                        clients[slot].active = 0;
                        continue;
                    }

                    char ack[MAX_MSG_SIZE];
                    sprintf(ack, "OK|%d", clients[slot].id);
                    mq_send(clients[slot].queue, ack, strlen(ack) + 1, 1);

                    char join_msg[MAX_MSG_SIZE];
                    sprintf(join_msg, "[SERVER] %s joined the chat", name);
                    send_to_all(join_msg, clients[slot].id, 0);
                    broadcast_user_list();
                }
            }
            else if (strcmp(cmd, "MSG") == 0) {
                int sender_id;
                char msg[MAX_MSG_SIZE];
                sscanf(data, "%d|%[^\n]", &sender_id, msg);

                int sender_slot = find_client_by_id(sender_id);
                if (sender_slot >= 0) {
                    char formatted[MAX_MSG_SIZE];
                    char temp[MAX_MSG_SIZE];
                    strcpy(temp, "[");
                    strcat(temp, clients[sender_slot].name);
                    strcat(temp, "] ");
                    strcat(temp, msg);
                    strcpy(formatted, temp);
                    send_to_all(formatted, sender_id, 0);
                }
            }
            else if (strcmp(cmd, "QUIT") == 0) {
                int slot_id;
                sscanf(data, "%d", &slot_id);

                int slot = find_client_by_id(slot_id);
                if (slot >= 0 && clients[slot].active) {
                    char leave_msg[MAX_MSG_SIZE];
                    sprintf(leave_msg, "[SERVER] %s left the chat", clients[slot].name);
                    clients[slot].active = 0;
                    mq_close(clients[slot].queue);
                    mq_unlink(clients[slot].queue_name);

                    send_to_all(leave_msg, slot_id, 0);
                    broadcast_user_list();
                }
            }
        }
    }
    return NULL;
}

int main() {
    signal(SIGINT, cleanup);

    mq_unlink(SERVER_QUEUE);

    struct mq_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;

    server_q = mq_open(SERVER_QUEUE, O_CREAT | O_RDWR, 0666, &attr);
    if (server_q == -1) {
        perror("mq_open server");
        exit(1);
    }

    memset(clients, 0, sizeof(clients));

    pthread_t handler_thread;
    pthread_create(&handler_thread, NULL, handle_clients, NULL);

    printf("Chat server started\n");
    printf("Queue: %s\n", SERVER_QUEUE);

    pthread_join(handler_thread, NULL);
    cleanup();

    return 0;
}