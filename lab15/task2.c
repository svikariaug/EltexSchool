#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main() {
    sigset_t block_set, old_set;

    sigemptyset(&block_set);
    sigaddset(&block_set, SIGINT);

    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) == -1) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    printf("Программа запущена. PID = %d\n", getpid());
    printf("SIGINT заблокирован. Нажмите Ctrl+C или отправьте SIGINT — он не подействует.\n");
    printf("Для завершения отправьте SIGTERM или SIGKILL (kill -9 %d)\n", getpid());

    int count = 0;
    while (1) {
        sleep(2);
        printf("Прошло %d секунд... (SIGINT игнорируется)\n", count += 2);
        fflush(stdout);
    }

    return 0;
}