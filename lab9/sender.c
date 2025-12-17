#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678
#define BUF_SIZE 128

struct sembuf lock = {0, -1, 0};
struct sembuf unlock = {0, 1, 0};

int main() {
    int shmid = shmget(SHM_KEY, BUF_SIZE, IPC_CREAT | 0666);
    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);

    semctl(semid, 0, SETVAL, 1);

    char* shm = shmat(shmid, NULL, 0);

    while (1) {
        semop(semid, &lock, 1);

        time_t now = time(NULL);
        snprintf(shm, BUF_SIZE, "Отправитель pid=%d, time=%s",
                 getpid(), ctime(&now));

        semop(semid, &unlock, 1);
        sleep(3);
    }
}
