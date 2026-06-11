#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main() {
    sigset_t wait_set;
    int sig;

    sigemptyset(&wait_set);
    sigaddset(&wait_set, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &wait_set, NULL) == -1) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    printf("Программа запущена. PID = %d\n", getpid());
    printf("Ожидание SIGUSR1 через sigwait().\n");

    while (1) {
        if (sigwait(&wait_set, &sig) != 0) {
            perror("sigwait");
            exit(EXIT_FAILURE);
        }
        printf("Получен сигнал %d (SIGUSR1)\n", sig);
        fflush(stdout);
    }

    return 0;
}