#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64

// Функция для разбора командной строки на отдельные команды (разделенные '|')
int parse_pipeline(char *input, char *commands[], int max_commands) {
    int count = 0;
    char *token;
    
    token = strtok(input, "|");
    while (token != NULL && count < max_commands) {
        // Удаляем пробелы в начале и конце
        while (*token == ' ') token++;
        char *end = token + strlen(token) - 1;
        while (end > token && *end == ' ') end--;
        *(end + 1) = '\0';
        
        commands[count++] = token;
        token = strtok(NULL, "|");
    }
    
    return count;
}

// Функция для разбора команды на аргументы
int parse_command(char *cmd, char *args[]) {
    int count = 0;
    char *token;
    
    token = strtok(cmd, " \t");
    while (token != NULL && count < MAX_ARGS - 1) {
        args[count++] = token;
        token = strtok(NULL, " \t");
    }
    args[count] = NULL;
    
    return count;
}

// Выполнение конвейера команд
void execute_pipeline(char *commands[], int num_commands) {
    int pipes[2];
    int prev_pipe = -1;
    pid_t pid;
    
    for (int i = 0; i < num_commands; i++) {
        // Создаем пайп для следующей команды (кроме последней)
        if (i < num_commands - 1) {
            if (pipe(pipes) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }
        
        pid = fork();
        
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        
        if (pid == 0) {
            // Дочерний процесс
            char *args[MAX_ARGS];
            parse_command(commands[i], args);
            
            // Перенаправляем ввод из предыдущего пайпа
            if (prev_pipe != -1) {
                dup2(prev_pipe, STDIN_FILENO);
                close(prev_pipe);
            }
            
            // Перенаправляем вывод в следующий пайп
            if (i < num_commands - 1) {
                dup2(pipes[1], STDOUT_FILENO);
                close(pipes[0]);
                close(pipes[1]);
            }
            
            // Выполняем команду
            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
        
        // Родительский процесс
        if (prev_pipe != -1) {
            close(prev_pipe);
        }
        
        if (i < num_commands - 1) {
            close(pipes[1]);
            prev_pipe = pipes[0];
        }
    }
    
    // Ожидаем завершения всех дочерних процессов
    for (int i = 0; i < num_commands; i++) {
        wait(NULL);
    }
}

int main() {
    char input[MAX_CMD_LEN];
    char *commands[MAX_ARGS];
    int num_commands;
    
    printf("Простой командный интерпретатор\n");
    printf("Поддерживает конвейер (|)\n");
    printf("Для выхода введите 'exit' или 'quit'\n\n");
    
    while (1) {
        printf("$ ");
        fflush(stdout);
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        // Удаляем символ новой строки
        input[strcspn(input, "\n")] = '\0';
        
        // Проверяем команду выхода
        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) {
            printf("Выход из интерпретатора\n");
            break;
        }
        
        // Пропускаем пустые строки
        if (strlen(input) == 0) {
            continue;
        }
        
        // Разбираем конвейер
        num_commands = parse_pipeline(input, commands, MAX_ARGS);
        
        if (num_commands == 1) {
            // Простая команда без пайпов
            char *args[MAX_ARGS];
            parse_command(commands[0], args);
            
            pid_t pid = fork();
            
            if (pid == -1) {
                perror("fork");
                continue;
            }
            
            if (pid == 0) {
                execvp(args[0], args);
                perror("execvp");
                exit(EXIT_FAILURE);
            } else {
                wait(NULL);
            }
        } else {
            // Конвейер из нескольких команд
            execute_pipeline(commands, num_commands);
        }
    }
    
    return 0;
}