#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

int main() {
    int fd[2];

    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid > 0) {
        close(fd[0]); 

        sleep(8);


        time_t now = time(NULL);
        char *curr_time = ctime(&now);

        char msg[256];
        snprintf(msg, sizeof(msg),
                 "сообщение от родителя: PID=%d, время=%s",
                 getpid(), curr_time);

        write(fd[1], msg, strlen(msg) + 1);
        close(fd[1]);

    } else {
        close(fd[1]); 


        time_t now = time(NULL);
        char *child_time = ctime(&now);

        char buffer[256];
        read(fd[0], buffer, sizeof(buffer));
        close(fd[0]);

        printf("дочерний процесс:\n");
        printf("  текущее время: %s", child_time);
        printf("  получено: %s\n", buffer);
    }

    return 0;
}
