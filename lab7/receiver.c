#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <semaphore.h>
#include <string.h>

#define SHM_NAME "/time_shm"
#define SEM_NAME "/time_sem"

typedef struct {
    pid_t pid;
    char time_str[64];
} shm_data;

shm_data *data;
sem_t *sem;

void cleanup(int signo) {
    printf("\nзавершение приемника...\n");
    munmap(data, sizeof(shm_data));
    sem_close(sem);
    exit(0);
}


int main() {
    signal(SIGINT, cleanup);
    
    // открытие памяти
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd < 0) {
        perror("shm_open");
        return 1;
    }

    shm_data *data = mmap(NULL, sizeof(shm_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // открытие semaphore
    sem_t *sem = sem_open(SEM_NAME, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    while (1) {
        sem_wait(sem);
        pid_t sender_pid = data->pid;
        char sender_time[64];
        strcpy(sender_time, data->time_str);
        sem_post(sem);

        time_t t = time(NULL);
        char my_time[64];
        strftime(my_time, sizeof(my_time), "%H:%M:%S", localtime(&t));

        printf("receive PID: %d, время: %s, received -> PID: %d, время: %s\n",
               getpid(), my_time, sender_pid, sender_time);

        sleep(1);
    }

    return 0;
}
