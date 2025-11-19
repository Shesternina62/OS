#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

int main() {

    sleep(13);
    
    int fd = open("myfifo", O_WRONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    time_t now = time(NULL);
    char *curr_time = ctime(&now);

    char msg[256];
    snprintf(msg, sizeof(msg),
             "PID=%d, время=%s",
             getpid(), curr_time);

    write(fd, msg, strlen(msg) + 1);
    close(fd);

    printf("oтправлено через fifo: %s\n", msg);

    return 0;
}
