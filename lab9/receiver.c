#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <time.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678

struct sembuf lock = {0, -1, 0};
struct sembuf unlock = {0, 1, 0};

int main() {
    int shmid = shmget(SHM_KEY, 0, 0666);
    int semid = semget(SEM_KEY, 1, 0666);

    char* shm = shmat(shmid, NULL, 0);

    while (1) {
        semop(semid, &lock, 1);

        time_t now = time(NULL);
        printf("Получатель pid=%d, time=%s", getpid(), ctime(&now));
        printf("Принято: %s\n", shm);

        semop(semid, &unlock, 1);
        sleep(3);
    }
}
