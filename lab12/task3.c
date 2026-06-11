#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64

int parse_pipeline(char *input, char *commands[], int max_commands) {
    int count = 0;
    char *token;
    
    token = strtok(input, "|");
    while (token != NULL && count < max_commands) {
        while (*token == ' ') token++;
        char *end = token + strlen(token) - 1;
        while (end > token && *end == ' ') end--;
        *(end + 1) = '\0';
        
        commands[count++] = token;
        token = strtok(NULL, "|");
    }
    
    return count;
}

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

void execute_pipeline(char *commands[], int num_commands) {
    int pipes[2];
    int prev_pipe = -1;
    pid_t pid;
    
    for (int i = 0; i < num_commands; i++) {
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
            char *args[MAX_ARGS];
            parse_command(commands[i], args);

            if (prev_pipe != -1) {
                dup2(prev_pipe, STDIN_FILENO);
                close(prev_pipe);
            }

            if (i < num_commands - 1) {
                dup2(pipes[1], STDOUT_FILENO);
                close(pipes[0]);
                close(pipes[1]);
            }

            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }

        if (prev_pipe != -1) {
            close(prev_pipe);
        }
        
        if (i < num_commands - 1) {
            close(pipes[1]);
            prev_pipe = pipes[0];
        }
    }

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

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) {
            printf("Выход из интерпретатора\n");
            break;
        }
 
        if (strlen(input) == 0) {
            continue;
        }

        num_commands = parse_pipeline(input, commands, MAX_ARGS);
        
        if (num_commands == 1) {
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
            execute_pipeline(commands, num_commands);
        }
    }
    
    return 0;
}