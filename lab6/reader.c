#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

int main() {
    int fd = open("myfifo", O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    time_t now = time(NULL);
    char *curr_time = ctime(&now);

    char buffer[256];

    read(fd, buffer, sizeof(buffer));
    close(fd);

    printf("получатель:\n");
    printf("текущее время до чтения: %s", curr_time);
    printf("получено через fifo: %s\n", buffer);

    return 0;
}
