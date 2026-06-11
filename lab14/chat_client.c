#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <signal.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>

#define SHM_NAME "/chat_shm"
#define SEM_MUTEX "/chat_mutex"
#define SEM_MSG_COUNT "/chat_msg_count"
#define MAX_CLIENTS 20
#define MAX_MSG_SIZE 512
#define MAX_NAME_LEN 32
#define MAX_MESSAGES 100

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    int active;
    time_t join_time;
} Client;

typedef struct {
    int id;
    int timestamp;
    char sender_name[MAX_NAME_LEN];
    char content[MAX_MSG_SIZE];
} Message;

typedef struct {
    int client_count;
    int next_id;
    int msg_count;
    int msg_head;
    int msg_tail;
    Client clients[MAX_CLIENTS];
    Message messages[MAX_MESSAGES];
    char user_list[4096];
} ChatShared;

static ChatShared *shared = NULL;
static int shm_fd;
static sem_t *mutex;
static sem_t *msg_count;
static int client_id = -1;
static volatile sig_atomic_t running = 1;
static char my_name[MAX_NAME_LEN];
static int registration_done = 0;

static WINDOW *msg_win = NULL;
static WINDOW *input_win = NULL;
static WINDOW *users_win = NULL;
static int msg_lines = 0;
static int processed_count = 0;

static void update_user_list(void);
static void add_message(const char *sender, const char *msg);
static void cleanup(void);

static void update_user_list(void) {
    if (!users_win || !shared || !mutex) return;
    
    int maxy, maxx;
    getmaxyx(stdscr, maxy, maxx);
    
    werase(users_win);
    box(users_win, 0, 0);
    mvwprintw(users_win, 0, 2, " Users ");
    
    sem_wait(mutex);
    char temp[4096];
    strcpy(temp, shared->user_list);
    sem_post(mutex);
    
    int line = 1;
    char *token = strtok(temp, ",");
    while (token != NULL && line < maxy - 2) {
        mvwprintw(users_win, line, 2, "%s", token);
        line++;
        token = strtok(NULL, ",");
    }
    
    wrefresh(users_win);
}

static void add_message(const char *sender, const char *msg) {
    if (!msg_win) return;
    
    int maxy, maxx;
    getmaxyx(msg_win, maxy, maxx);
    
    if (msg_lines >= maxy - 3) {
        scroll(msg_win);
        msg_lines = maxy - 4;
    }
    
    wmove(msg_win, msg_lines + 1, 2);
    
    char formatted[MAX_MSG_SIZE + MAX_NAME_LEN + 10];
    if (strcmp(sender, "SYSTEM") == 0) {
        snprintf(formatted, sizeof(formatted), "[%s] %s", sender, msg);
    } else {
        snprintf(formatted, sizeof(formatted), "[%s]: %s", sender, msg);
    }
    
    if (strlen(formatted) > (size_t)(maxx - 4)) {
        char temp[MAX_MSG_SIZE + MAX_NAME_LEN + 10];
        strncpy(temp, formatted, maxx - 7);
        temp[maxx - 7] = '\0';
        strcat(temp, "...");
        wprintw(msg_win, "%s", temp);
    } else {
        wprintw(msg_win, "%s", formatted);
    }
    
    msg_lines++;
    wrefresh(msg_win);
}

static void cleanup(void) {
    running = 0;
    
    if (shared && client_id >= 0 && mutex != NULL && mutex != SEM_FAILED) {
        sem_wait(mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (shared->clients[i].active && shared->clients[i].id == client_id) {
                shared->clients[i].active = 0;
                shared->client_count--;
                
                char leave_msg[MAX_MSG_SIZE];
                snprintf(leave_msg, sizeof(leave_msg), "%s left the chat", my_name);
                
                int next = (shared->msg_tail + 1) % MAX_MESSAGES;
                if (next != shared->msg_head) {
                    strcpy(shared->messages[shared->msg_tail].sender_name, "SYSTEM");
                    strcpy(shared->messages[shared->msg_tail].content, leave_msg);
                    shared->messages[shared->msg_tail].timestamp = (int)time(NULL);
                    shared->msg_tail = next;
                    shared->msg_count++;
                    sem_post(msg_count);
                }
                break;
            }
        }
        sem_post(mutex);
    }
    
    if (shared) {
        munmap(shared, sizeof(ChatShared));
    }
    
    if (shm_fd > 0) {
        close(shm_fd);
    }
    
    if (mutex != NULL && mutex != SEM_FAILED) {
        sem_close(mutex);
    }
    
    if (msg_count != NULL && msg_count != SEM_FAILED) {
        sem_close(msg_count);
    }
    
    if (msg_win) delwin(msg_win);
    if (users_win) delwin(users_win);
    if (input_win) delwin(input_win);
    
    endwin();
}

static void* receive_messages(void *arg) {
    (void)arg;
    
    while (running) {
        if (shared && mutex != NULL && mutex != SEM_FAILED && 
            msg_count != NULL && msg_count != SEM_FAILED) {
            
            sem_wait(mutex);
            int current_count = shared->msg_count;
            int current_tail = shared->msg_tail;
            sem_post(mutex);
            
            while (processed_count < current_count && running) {
                sem_wait(mutex);
                int read_pos = (shared->msg_head + processed_count) % MAX_MESSAGES;
                
                if (read_pos != current_tail) {
                    Message msg = shared->messages[read_pos];
                    add_message(msg.sender_name, msg.content);
                    processed_count++;
                }
                sem_post(mutex);
            }
            
            update_user_list();
        }
        
        usleep(50000);
    }
    
    return NULL;
}

static void* send_messages(void *arg) {
    (void)arg;
    char input[MAX_MSG_SIZE];
    int ch, pos = 0;

    while (running && !registration_done) {
        usleep(100000);
    }
    
    while (running) {
        int maxx;
        getmaxyx(stdscr, maxx, maxx);
        
        if (!input_win) {
            usleep(100000);
            continue;
        }
        
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
                    
                    if (shared && mutex != NULL && mutex != SEM_FAILED) {
                        sem_wait(mutex);
                        int next = (shared->msg_tail + 1) % MAX_MESSAGES;
                        if (next != shared->msg_head) {
                            shared->messages[shared->msg_tail].id = client_id;
                            strcpy(shared->messages[shared->msg_tail].sender_name, my_name);
                            strcpy(shared->messages[shared->msg_tail].content, input);
                            shared->messages[shared->msg_tail].timestamp = (int)time(NULL);
                            shared->msg_tail = next;
                            shared->msg_count++;
                            sem_post(msg_count);
                        }
                        sem_post(mutex);
                    }
                }
                break;
            }
            else if (ch == 27) {
                running = 0;
                cleanup();
                pthread_exit(NULL);
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
                input[pos++] = (char)ch;
                input[pos] = '\0';
                mvwprintw(input_win, 1, 2, "%s", input);
                wrefresh(input_win);
            }
        }
    }
    
    return NULL;
}

int main(void) {
    printf("Enter your name: ");
    fflush(stdout);
    fgets(my_name, MAX_NAME_LEN, stdin);
    my_name[strcspn(my_name, "\n")] = '\0';
    
    if (strlen(my_name) == 0) {
        printf("Name cannot be empty!\n");
        exit(1);
    }
    
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        printf("Make sure server is running: ./chat_server\n");
        exit(1);
    }
    
    shared = mmap(0, sizeof(ChatShared), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    
    mutex = sem_open(SEM_MUTEX, 0);
    msg_count = sem_open(SEM_MSG_COUNT, 0);
    
    if (mutex == SEM_FAILED || msg_count == SEM_FAILED) {
        perror("sem_open");
        printf("Make sure server is running\n");
        cleanup();
        exit(1);
    }

    sem_wait(mutex);

    int already_exists = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (shared->clients[i].active && strcmp(shared->clients[i].name, my_name) == 0) {
            already_exists = 1;
            break;
        }
    }
    
    if (already_exists) {
        sem_post(mutex);
        printf("User '%s' is already in the chat!\n", my_name);
        cleanup();
        exit(1);
    }
    
    int slot = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!shared->clients[i].active) {
            slot = i;
            break;
        }
    }
    
    if (slot >= 0) {
        client_id = shared->next_id++;
        strcpy(shared->clients[slot].name, my_name);
        shared->clients[slot].id = client_id;
        shared->clients[slot].active = 1;
        shared->clients[slot].join_time = time(NULL);
        shared->client_count++;

        char temp[4096] = "";
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (shared->clients[i].active) {
                if (strlen(temp) > 0) {
                    strcat(temp, ",");
                }
                strcat(temp, shared->clients[i].name);
            }
        }
        strcpy(shared->user_list, temp);

        char join_msg[MAX_MSG_SIZE];
        snprintf(join_msg, sizeof(join_msg), "%s joined the chat", my_name);
        
        int next = (shared->msg_tail + 1) % MAX_MESSAGES;
        if (next != shared->msg_head) {
            shared->messages[shared->msg_tail].id = -1;
            strcpy(shared->messages[shared->msg_tail].sender_name, "SYSTEM");
            strcpy(shared->messages[shared->msg_tail].content, join_msg);
            shared->messages[shared->msg_tail].timestamp = (int)time(NULL);
            shared->msg_tail = next;
            shared->msg_count++;
            sem_post(msg_count);
        }
        
        registration_done = 1;
    }
    sem_post(mutex);
    
    if (client_id == -1) {
        printf("Chat is full! Cannot connect.\n");
        cleanup();
        exit(1);
    }

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
    
    box(msg_win, 0, 0);
    wrefresh(msg_win);
    
    box(users_win, 0, 0);
    mvwprintw(users_win, 0, 2, " Users ");
    wrefresh(users_win);
    
    box(input_win, 0, 0);
    mvwprintw(input_win, 0, 2, " Enter message (ESC to exit) ");
    wrefresh(input_win);
    
    msg_lines = 0;
    processed_count = 0;

    add_message("SYSTEM", "Welcome to the chat!");
    add_message("SYSTEM", "You are logged in as:");
    add_message("SYSTEM", my_name);
    
    pthread_t recv_thread, send_thread;
    pthread_create(&recv_thread, NULL, receive_messages, NULL);
    pthread_create(&send_thread, NULL, send_messages, NULL);
    
    pthread_join(recv_thread, NULL);
    pthread_join(send_thread, NULL);
    
    cleanup();
    return 0;
}