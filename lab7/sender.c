#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>

#define SHM_NAME "/time_shm"
#define SEM_NAME "/time_sem"

typedef struct {
    pid_t pid;
    char time_str[64];
} shm_data;

int fd_lock;
int shm_fd;
shm_data *data;
sem_t *sem;

void cleanup(int signo) {
    printf("\nзавершение отправителя...\n");
    munmap(data, sizeof(shm_data));
    close(shm_fd);
    shm_unlink(SHM_NAME);
    sem_close(sem);
    sem_unlink(SEM_NAME);
    close(fd_lock);
    unlink("/tmp/sender.lock");
    exit(0);
}

int main() {
    signal(SIGINT, cleanup);

    // проверка,начат ли процесс
    int fd_lock = open("/tmp/sender.lock", O_CREAT | O_RDWR, 0666);
    if (fd_lock < 0) {
        perror("open lock");
        return 1;
    }
    if (flock(fd_lock, LOCK_EX | LOCK_NB) < 0) {
        printf("отправитель запущен!\n");
        return 1;
    }

    // cоздание памяти
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd < 0) {
        perror("shm_open");
        return 1;
    }
    ftruncate(shm_fd, sizeof(shm_data));

    shm_data *data = mmap(NULL, sizeof(shm_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // создание semaphore
    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    while (1) {
        time_t t = time(NULL);
        char buf[64];
        strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&t));

        sem_wait(sem);
        data->pid = getpid();
        strcpy(data->time_str, buf);
        sem_post(sem);

        sleep(1);
    }

    return 0;
}
