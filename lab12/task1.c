#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main() {
    int pipefd[2];
    pid_t pid;
    
    // Создаем неименованный канал
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    
    // Создаем процесс
    pid = fork();
    
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        // Дочерний процесс
        close(pipefd[1]); // Закрываем дескриптор для записи
        
        char buffer[100];
        ssize_t bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1);
        
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Дочерний процесс прочитал: %s\n", buffer);
        }
        
        close(pipefd[0]);
        exit(EXIT_SUCCESS);
    } else {
        // Родительский процесс
        close(pipefd[0]); // Закрываем дескриптор для чтения
        
        char message[] = "Hi!";
        write(pipefd[1], message, strlen(message) + 1);
        printf("Родительский процесс записал: %s\n", message);
        
        close(pipefd[1]);
        
        // Ожидаем завершения дочернего процесса
        wait(NULL);
        printf("Дочерний процесс завершен\n");
    }
    
    return 0;
}