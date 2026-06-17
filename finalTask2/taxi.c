#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>

typedef struct {
    pid_t pid;
    int read_fd;
    int write_fd;
    int busy;
    time_t end_time;
} Driver;

Driver *drivers = NULL;
int num_drivers = 0;
int max_drivers = 0;
int expected_pid = 0;

int find_driver(pid_t pid) {
    for (int i = 0; i < num_drivers; i++) {
        if (drivers[i].pid == pid)
            return i;
    }
    return -1;
}

void remove_driver(int idx) {
    if (idx < 0 || idx >= num_drivers) return;
    close(drivers[idx].read_fd);
    close(drivers[idx].write_fd);
    for (int i = idx; i < num_drivers - 1; i++) {
        drivers[i] = drivers[i + 1];
    }
    num_drivers--;
}

void add_driver(pid_t pid, int read_fd, int write_fd) {
    if (num_drivers >= max_drivers) {
        max_drivers = max_drivers ? max_drivers * 2 : 4;
        drivers = realloc(drivers, max_drivers * sizeof(Driver));
        if (!drivers) {
            perror("realloc");
            exit(1);
        }
    }
    drivers[num_drivers].pid = pid;
    drivers[num_drivers].read_fd = read_fd;
    drivers[num_drivers].write_fd = write_fd;
    drivers[num_drivers].busy = 0;
    drivers[num_drivers].end_time = 0;
    num_drivers++;
}

int read_line(int fd, char *buf, size_t size) {
    size_t i = 0;
    char c;
    while (i < size - 1) {
        ssize_t n = read(fd, &c, 1);
        if (n <= 0) {
            if (n == 0) break;
            if (errno == EINTR) continue;
            return -1;
        }
        if (c == '\n') break;
        buf[i++] = c;
    }
    buf[i] = '\0';
    return i;
}

char *wait_for_response(int idx) {
    if (idx < 0 || idx >= num_drivers) return NULL;
    Driver *d = &drivers[idx];
    expected_pid = d->pid;
    char *response = NULL;

    while (1) {
        fd_set fds;
        FD_ZERO(&fds);
        int maxfd = -1;
        for (int i = 0; i < num_drivers; i++) {
            FD_SET(drivers[i].read_fd, &fds);
            if (drivers[i].read_fd > maxfd) maxfd = drivers[i].read_fd;
        }

        int ret = select(maxfd + 1, &fds, NULL, NULL, NULL);
        if (ret < 0) {
            perror("wait_for_response select");
            break;
        }

        for (int i = 0; i < num_drivers; i++) {
            if (FD_ISSET(drivers[i].read_fd, &fds)) {
                char buf[256];
                int n = read_line(drivers[i].read_fd, buf, sizeof(buf));
                if (n < 0) {
                    remove_driver(i);
                    i--;
                    continue;
                }
                if (n == 0) {
                    remove_driver(i);
                    i--;
                    continue;
                }

                if (drivers[i].pid == expected_pid) {
                    response = strdup(buf);
                    expected_pid = 0;
                    if (strcmp(buf, "Available") == 0) {
                        drivers[i].busy = 0;
                    } else if (strncmp(buf, "Busy", 4) == 0) {
                        int rem;
                        if (sscanf(buf, "Busy %d", &rem) == 1) {
                            drivers[i].busy = 1;
                            drivers[i].end_time = time(NULL) + rem;
                        }
                    }
                    return response;
                } else {
                    if (strcmp(buf, "Available") == 0) {
                        drivers[i].busy = 0;
                    } else if (strncmp(buf, "Busy", 4) == 0) {
                        int rem;
                        if (sscanf(buf, "Busy %d", &rem) == 1) {
                            drivers[i].busy = 1;
                            drivers[i].end_time = time(NULL) + rem;
                        }
                    }
                }
            }
        }
    }
    expected_pid = 0;
    return response;
}

void driver_loop(int read_fd, int write_fd) {
    int busy = 0;
    time_t end_time = 0;
    char buf[256];

    while (1) {
        if (busy && time(NULL) >= end_time) {
            busy = 0;
            dprintf(write_fd, "Available\n");
        }

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(read_fd, &fds);
        struct timeval tv;
        struct timeval *tvp = NULL;
        if (busy) {
            long rem = end_time - time(NULL);
            if (rem < 0) rem = 0;
            tv.tv_sec = rem;
            tv.tv_usec = 0;
            tvp = &tv;
        }

        int ret = select(read_fd + 1, &fds, NULL, NULL, tvp);
        if (ret < 0) {
            perror("driver select");
            break;
        }
        if (ret == 0) {
            continue;
        }

        if (FD_ISSET(read_fd, &fds)) {
            int n = read_line(read_fd, buf, sizeof(buf));
            if (n <= 0) break;

            if (strncmp(buf, "task", 4) == 0) {
                int timer;
                if (sscanf(buf, "task %d", &timer) == 1) {
                    if (busy) {
                        long rem = end_time - time(NULL);
                        if (rem < 0) rem = 0;
                        dprintf(write_fd, "Busy %ld\n", rem);
                    } else {
                        busy = 1;
                        end_time = time(NULL) + timer;
                        dprintf(write_fd, "OK\n");
                    }
                } else {
                    dprintf(write_fd, "ERROR\n");
                }
            } else if (strcmp(buf, "status") == 0) {
                if (busy) {
                    long rem = end_time - time(NULL);
                    if (rem < 0) rem = 0;
                    dprintf(write_fd, "Busy %ld\n", rem);
                } else {
                    dprintf(write_fd, "Available\n");
                }
            } else {
                dprintf(write_fd, "UNKNOWN\n");
            }
        }
    }

    close(read_fd);
    close(write_fd);
    exit(0);
}

int main() {
    signal(SIGCHLD, SIG_IGN);

    printf("Taxi Dispatcher CLI\n");
    printf("Commands: create_driver, send_task <pid> <timer>, get_status <pid>, get_drivers\n");

    char cmd[256];
    while (1) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        int maxfd = STDIN_FILENO;
        for (int i = 0; i < num_drivers; i++) {
            FD_SET(drivers[i].read_fd, &fds);
            if (drivers[i].read_fd > maxfd) maxfd = drivers[i].read_fd;
        }

        int ret = select(maxfd + 1, &fds, NULL, NULL, NULL);
        if (ret < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &fds)) {
            if (fgets(cmd, sizeof(cmd), stdin) == NULL) break;
            cmd[strcspn(cmd, "\n")] = '\0';

            char *token = strtok(cmd, " ");
            if (token == NULL) continue;

            if (strcmp(token, "create_driver") == 0) {
                int cmd_pipe[2], resp_pipe[2];
                if (pipe(cmd_pipe) == -1 || pipe(resp_pipe) == -1) {
                    perror("pipe");
                    continue;
                }

                pid_t pid = fork();
                if (pid == -1) {
                    perror("fork");
                    continue;
                }

                if (pid == 0) {
                    close(cmd_pipe[1]);
                    close(resp_pipe[0]);
                    driver_loop(cmd_pipe[0], resp_pipe[1]);
                } else {
                    close(cmd_pipe[0]);
                    close(resp_pipe[1]);
                    add_driver(pid, resp_pipe[0], cmd_pipe[1]);
                    printf("Driver created with PID %d\n", pid);
                }
            } else if (strcmp(token, "send_task") == 0) {
                char *pid_str = strtok(NULL, " ");
                char *timer_str = strtok(NULL, " ");
                if (!pid_str || !timer_str) {
                    printf("Usage: send_task <pid> <timer>\n");
                    continue;
                }
                pid_t pid = atoi(pid_str);
                int timer = atoi(timer_str);
                int idx = find_driver(pid);
                if (idx == -1) {
                    printf("Driver not found\n");
                } else {
                    dprintf(drivers[idx].write_fd, "task %d\n", timer);
                    char *resp = wait_for_response(idx);
                    if (resp) {
                        if (strncmp(resp, "OK", 2) == 0) {
                            drivers[idx].busy = 1;
                            drivers[idx].end_time = time(NULL) + timer;
                            printf("Task assigned\n");
                        } else {
                            printf("%s\n", resp);
                        }
                        free(resp);
                    }
                }
            } else if (strcmp(token, "get_status") == 0) {
                char *pid_str = strtok(NULL, " ");
                if (!pid_str) {
                    printf("Usage: get_status <pid>\n");
                    continue;
                }
                pid_t pid = atoi(pid_str);
                int idx = find_driver(pid);
                if (idx == -1) {
                    printf("Driver not found\n");
                } else {
                    dprintf(drivers[idx].write_fd, "status\n");
                    char *resp = wait_for_response(idx);
                    if (resp) {
                        printf("%s\n", resp);
                        if (strcmp(resp, "Available") == 0) {
                            drivers[idx].busy = 0;
                        } else if (strncmp(resp, "Busy", 4) == 0) {
                            int rem;
                            if (sscanf(resp, "Busy %d", &rem) == 1) {
                                drivers[idx].busy = 1;
                                drivers[idx].end_time = time(NULL) + rem;
                            }
                        }
                        free(resp);
                    }
                }
            } else if (strcmp(token, "get_drivers") == 0) {
                for (int i = 0; i < num_drivers; i++) {
                    Driver *d = &drivers[i];
                    if (d->busy && time(NULL) >= d->end_time) {
                        d->busy = 0;
                    }
                    if (d->busy) {
                        long rem = d->end_time - time(NULL);
                        if (rem < 0) rem = 0;
                        printf("PID: %d, Status: Busy %ld\n", d->pid, rem);
                    } else {
                        printf("PID: %d, Status: Available\n", d->pid);
                    }
                }
            } else {
                printf("Unknown command\n");
            }
        }

        for (int i = 0; i < num_drivers; i++) {
            if (FD_ISSET(drivers[i].read_fd, &fds)) {
                char buf[256];
                int n = read_line(drivers[i].read_fd, buf, sizeof(buf));
                if (n < 0) {
                    remove_driver(i);
                    i--;
                    continue;
                }
                if (n == 0) {
                    remove_driver(i);
                    i--;
                    continue;
                }
                if (strcmp(buf, "Available") == 0) {
                    drivers[i].busy = 0;
                } else if (strncmp(buf, "Busy", 4) == 0) {
                    int rem;
                    if (sscanf(buf, "Busy %d", &rem) == 1) {
                        drivers[i].busy = 1;
                        drivers[i].end_time = time(NULL) + rem;
                    }
                }
            }
        }
    }

    for (int i = 0; i < num_drivers; i++) {
        close(drivers[i].read_fd);
        close(drivers[i].write_fd);
    }
    free(drivers);
    return 0;
}