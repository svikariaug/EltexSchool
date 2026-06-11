#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void sigusr1_handler(int signo) {
    printf("Получен сигнал SIGUSR1 (номер %d)\n", signo);
    fflush(stdout);
}

int main() {
    struct sigaction sa;

    sa.sa_handler = sigusr1_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; 

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    printf("Программа запущена. PID = %d\n", getpid());
    printf("Ожидание сигнала SIGUSR1. Для выхода нажмите Ctrl+C\n");

    while (1) {
        pause();
    }

    return 0;
}