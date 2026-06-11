#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <signal.h>
#include <errno.h>

#define SERVER_QUEUE "/chat_server"
#define MAX_MSG_SIZE 512
#define MAX_NAME_LEN 32

mqd_t server_q;
mqd_t client_q;
int client_id = -1;
int running = 1;
char name[MAX_NAME_LEN];

WINDOW *msg_win, *input_win, *users_win;
int msg_lines = 0;
char user_list[1024] = "";

void cleanup() {
    running = 0;
    if (client_id >= 0) {
        char quit_msg[MAX_MSG_SIZE];
        sprintf(quit_msg, "QUIT|%d", client_id);
        mq_send(server_q, quit_msg, strlen(quit_msg) + 1, 1);
        usleep(100000);
    }
    if (server_q > 0) mq_close(server_q);
    if (client_q > 0) mq_close(client_q);
    endwin();
}

void refresh_screen() {
    int maxy, maxx;
    getmaxyx(stdscr, maxy, maxx);
    
    int users_width = 20;
    int msg_width = maxx - users_width - 2;
    
    werase(msg_win);
    box(msg_win, 0, 0);
    wrefresh(msg_win);
    
    werase(users_win);
    box(users_win, 0, 0);
    mvwprintw(users_win, 0, 2, " Users ");
    wrefresh(users_win);
    
    werase(input_win);
    box(input_win, 0, 0);
    mvwprintw(input_win, 0, 2, " Enter message (ESC to exit) ");
    wrefresh(input_win);
    
    msg_lines = 0;
}

void update_users_window() {
    int maxy, maxx;
    getmaxyx(stdscr, maxy, maxx);
    
    werase(users_win);
    box(users_win, 0, 0);
    mvwprintw(users_win, 0, 2, " Users ");
    
    char temp[1024];
    strcpy(temp, user_list);
    
    int line = 1;
    char *token = strtok(temp, ",");
    while (token != NULL && line < maxy - 2) {
        mvwprintw(users_win, line, 2, "%s", token);
        line++;
        token = strtok(NULL, ",");
    }
    
    wrefresh(users_win);
}

void add_message(const char *msg) {
    int maxy, maxx;
    getmaxyx(msg_win, maxy, maxx);
    
    wmove(msg_win, msg_lines + 1, 2);
    
    if (strlen(msg) > (size_t)(maxx - 4)) {
        char temp[MAX_MSG_SIZE];
        strncpy(temp, msg, maxx - 7);
        temp[maxx - 7] = '\0';
        strcat(temp, "...");
        wprintw(msg_win, "%s", temp);
    } else {
        wprintw(msg_win, "%s", msg);
    }
    
    msg_lines++;
    
    if (msg_lines >= maxy - 2) {
        scroll(msg_win);
        msg_lines = maxy - 3;
    }
    wrefresh(msg_win);
}

void* receive_messages(void *arg) {
    char buffer[MAX_MSG_SIZE];
    unsigned int prio;
    
    (void)arg;
    
    while (running) {
        if (mq_receive(client_q, buffer, MAX_MSG_SIZE, &prio) > 0) {
            if (strncmp(buffer, "OK|", 3) == 0) {
                sscanf(buffer, "OK|%d", &client_id);
                add_message("[SYSTEM] Welcome to chat!");
            }
            else if (strncmp(buffer, "USER_LIST|", 10) == 0) {
                strcpy(user_list, buffer + 10);
                update_users_window();
            }
            else {
                add_message(buffer);
            }
        }
        usleep(10000);
    }
    return NULL;
}

void* send_messages(void *arg) {
    char input[MAX_MSG_SIZE];
    int ch, pos = 0;
    
    (void)arg;
    
    while (running && client_id == -1) {
        usleep(100000);
    }
    
    while (running) {
        int maxx;
        getmaxyx(stdscr, maxx, maxx);
        
        wattron(input_win, A_REVERSE);
        mvwprintw(input_win, 1, 2, "%-*s", maxx - 4, "");
        wmove(input_win, 1, 2);
        wrefresh(input_win);
        wattroff(input_win, A_REVERSE);
        
        pos = 0;
        memset(input, 0, sizeof(input));
        
        while (1) {
            ch = wgetch(input_win);
            if (ch == '\n') {
                if (pos > 0) {
                    input[pos] = '\0';
                    char msg[MAX_MSG_SIZE];
                    char temp[MAX_MSG_SIZE];
                    strcpy(temp, "MSG|");
                    char id_str[16];
                    sprintf(id_str, "%d", client_id);
                    strcat(temp, id_str);
                    strcat(temp, "|");
                    strcat(temp, input);
                    strcpy(msg, temp);
                    mq_send(server_q, msg, strlen(msg) + 1, 1);
                    
                    char local_msg[MAX_MSG_SIZE];
                    char local_temp[MAX_MSG_SIZE];
                    strcpy(local_temp, "[");
                    strcat(local_temp, name);
                    strcat(local_temp, "] ");
                    strcat(local_temp, input);
                    strcpy(local_msg, local_temp);
                    add_message(local_msg);
                }
                break;
            }
            else if (ch == 27) {
                running = 0;
                cleanup();
                exit(0);
            }
            else if (ch == KEY_BACKSPACE || ch == 127) {
                if (pos > 0) {
                    pos--;
                    input[pos] = '\0';
                    mvwprintw(input_win, 1, 2, "%-*s", maxx - 4, input);
                    wmove(input_win, 1, 2 + pos);
                    wrefresh(input_win);
                }
            }
            else if (ch >= 32 && ch <= 126 && pos < MAX_MSG_SIZE - 1) {
                input[pos++] = ch;
                input[pos] = '\0';
                mvwprintw(input_win, 1, 2, "%s", input);
                wrefresh(input_win);
            }
        }
    }
    return NULL;
}

int main() {
    printf("Enter your name: ");
    fflush(stdout);
    fgets(name, MAX_NAME_LEN, stdin);
    name[strcspn(name, "\n")] = 0;
    
    if (strlen(name) == 0) {
        printf("Name cannot be empty!\n");
        exit(1);
    }
    
    server_q = mq_open(SERVER_QUEUE, O_RDWR);
    if (server_q == -1) {
        perror("Error: server not running");
        printf("Make sure server is running: ./chat_server\n");
        exit(1);
    }
    
    char client_queue_name[64];
    sprintf(client_queue_name, "/chat_client_%d", getpid());
    
    struct mq_attr attr = {0, 10, MAX_MSG_SIZE, 0};
    client_q = mq_open(client_queue_name, O_CREAT | O_RDWR, 0666, &attr);
    if (client_q == -1) {
        perror("mq_open client");
        exit(1);
    }
    
    char reg_msg[MAX_MSG_SIZE];
    sprintf(reg_msg, "REG|%s|%s", name, client_queue_name);
    mq_send(server_q, reg_msg, strlen(reg_msg) + 1, 1);
    
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1);
    
    int maxy, maxx;
    getmaxyx(stdscr, maxy, maxx);
    
    int users_width = 20;
    int msg_width = maxx - users_width - 2;
    
    msg_win = newwin(maxy - 3, msg_width, 0, 0);
    users_win = newwin(maxy - 3, users_width, 0, msg_width + 1);
    input_win = newwin(3, maxx, maxy - 3, 0);
    
    scrollok(msg_win, TRUE);
    keypad(input_win, TRUE);
    refresh_screen();
    
    pthread_t recv_thread, send_thread;
    pthread_create(&recv_thread, NULL, receive_messages, NULL);
    pthread_create(&send_thread, NULL, send_messages, NULL);
    
    int wait_count = 0;
    while (client_id == -1 && wait_count < 50) {
        usleep(100000);
        wait_count++;
    }
    
    if (client_id == -1) {
        endwin();
        printf("Error: failed to register with server\n");
        cleanup();
        exit(1);
    }
    
    add_message("[SYSTEM] You are logged in as: ");
    add_message(name);
    
    pthread_join(recv_thread, NULL);
    pthread_join(send_thread, NULL);
    
    cleanup();
    return 0;
}