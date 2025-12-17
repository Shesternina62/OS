#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#define BUF_SIZE 64

char buffer[BUF_SIZE];
sem_t *sem;
volatile sig_atomic_t stop = 0;

void handle_sigint(int sig) {
    stop = 1;
}

void* writer(void* arg) {
    int counter = 0;

    while (!stop) {
        sem_wait(sem);

        snprintf(buffer, BUF_SIZE, "Запись № %d", counter++);
        sem_post(sem);

        sleep(1);
    }
    return NULL;
}

void* reader(void* arg) {
    pthread_t tid = pthread_self();

    while (!stop) {
        sem_wait(sem);

        printf("reader tid=%lu, buffer=\"%s\"\n",
               (unsigned long)tid, buffer);
        fflush(stdout);

        sem_post(sem);
        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t w, r;

    signal(SIGINT, handle_sigint);

    sem = sem_open("/lab9", O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    strcpy(buffer, "пусто");

    pthread_create(&w, NULL, writer, NULL);
    pthread_create(&r, NULL, reader, NULL);

    pthread_join(w, NULL);
    pthread_join(r, NULL);

    sem_close(sem);
    sem_unlink("/lab9_");

    printf("\nзавершено правильно\n");
    return 0;
}
