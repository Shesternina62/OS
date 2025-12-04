#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>

int main() {
    const char *fifo_name = "myfifo";
    if (access(fifo_name, F_OK) == -1) {
        if (mkfifo(fifo_name, 0666) == -1) {
            perror("mkfifo");
            exit(1);
        }
    }

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
