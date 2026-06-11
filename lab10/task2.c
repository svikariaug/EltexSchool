#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    pid_t pid1, pid2;

    pid1 = fork();

    if (pid1 < 0) {
        perror("fork1 failed");
        return 1;
    }

    if (pid1 == 0) {
        printf("Process 1: PID = %d, PPID = %d\n", getpid(), getppid());
        
        pid_t pid3 = fork();
        if (pid3 < 0) {
            perror("fork3 failed");
            exit(1);
        }
        if (pid3 == 0) {
            printf("Process 3: PID = %d, PPID = %d\n", getpid(), getppid());
            sleep(1);
            exit(0);
        } else {
            pid_t pid4 = fork();
            if (pid4 < 0) {
                perror("fork4 failed");
                exit(1);
            }
            if (pid4 == 0) {
                printf("Process 4: PID = %d, PPID = %d\n", getpid(), getppid());
                sleep(1);
                exit(0);
            } else {
                wait(NULL);
                wait(NULL);
                printf("Process 1: child processes 3 and 4 finished\n");
                exit(0);
            }
        }
    } else {
        pid2 = fork();
        if (pid2 < 0) {
            perror("fork2 failed");
            return 1;
        }
        if (pid2 == 0) {
            printf("Process 2: PID = %d, PPID = %d\n", getpid(), getppid());
            
            pid_t pid5 = fork();
            if (pid5 < 0) {
                perror("fork5 failed");
                exit(1);
            }
            if (pid5 == 0) {
                printf("Process 5: PID = %d, PPID = %d\n", getpid(), getppid());
                sleep(1);
                exit(0);
            } else {
                wait(NULL);
                printf("Process 2: child process 5 finished\n");
                exit(0);
            }
        } else {
            wait(NULL);
            wait(NULL);
            printf("Main parent: all processes finished\n");
        }
    }
    return 0;
}