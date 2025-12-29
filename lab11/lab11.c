#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define READERS_COUNT 10
#define BUFFER_SIZE 128
#define WORK_TIME 10

char shared_buffer[BUFFER_SIZE];
int counter = 0;
volatile int running = 1;
int new_data = 0;

pthread_mutex_t mutex;
pthread_cond_t cond;

void* writer_thread(void* arg) {
    while (running) {
        pthread_mutex_lock(&mutex);

        counter++;
        snprintf(shared_buffer, BUFFER_SIZE, "запись номер: %d", counter);
        new_data = 1;

        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);

        sleep(1);
    }
    return NULL;
}

void* reader_thread(void* arg) {
    long tid = (long)arg;

    while (running) {
        pthread_mutex_lock(&mutex);

        while (!new_data && running) {
            pthread_cond_wait(&cond, &mutex);
        }

        if (!running) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        printf("читатель %ld: %s\n", tid, shared_buffer);
        new_data = 0;

        pthread_mutex_unlock(&mutex);

        usleep(500000);
    }

    return NULL;
}

int main() {
    pthread_t readers[READERS_COUNT], writer;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    strcpy(shared_buffer, "начальное значение");

    pthread_create(&writer, NULL, writer_thread, NULL);

    for (long i = 0; i < READERS_COUNT; i++) {
        pthread_create(&readers[i], NULL, reader_thread, (void*)i);
    }

    sleep(WORK_TIME);
    running = 0;

    // сигналы
    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    pthread_join(writer, NULL);
    for (int i = 0; i < READERS_COUNT; i++) {
        pthread_join(readers[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    printf("программа завершена корректно\n");
    return 0;
}
