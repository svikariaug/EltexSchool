#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define FIFO_NAME "my_fifo"

int main() {
    int fd;
    char buffer[128];

    fd = open(FIFO_NAME, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    read(fd, buffer, sizeof(buffer));
    printf("Client received: %s\n", buffer);

    close(fd);
    return 0;
}