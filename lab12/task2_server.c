#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define FIFO_NAME "my_fifo"

int main() {
    int fd;

    unlink(FIFO_NAME);

    if (mkfifo(FIFO_NAME, 0666) == -1) {
        perror("mkfifo");
        exit(1);
    }

    printf("Server: FIFO created, waiting for client...\n");

    fd = open(FIFO_NAME, O_WRONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    write(fd, "Hi!", 4);
    close(fd);

    printf("Server: message sent\n");
    unlink(FIFO_NAME);
    
    return 0;
}