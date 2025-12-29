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

pthread_rwlock_t rwlock;
volatile int running = 1; 

void* writer_thread(void* arg) {
    while (running) {
        pthread_rwlock_wrlock(&rwlock);

        counter++;
        snprintf(shared_buffer, BUFFER_SIZE, "запись номер: %d", counter);

        pthread_rwlock_unlock(&rwlock);

        sleep(1); 
    }
    return NULL;
}

void* reader_thread(void* arg) {
    long tid = (long)arg;

    while (running) {
        pthread_rwlock_rdlock(&rwlock);

        printf("читатель %ld: %s\n", tid, shared_buffer);

        pthread_rwlock_unlock(&rwlock);

        usleep(500000); 
    }
    return NULL;
}

int main() {
    pthread_t readers[READERS_COUNT];
    pthread_t writer;

    pthread_rwlock_init(&rwlock, NULL);

    strcpy(shared_buffer, "начальное значение");

    // создание писателя
    pthread_create(&writer, NULL, writer_thread, NULL);

    // создание читателей
    for (long i = 0; i < READERS_COUNT; i++) {
        pthread_create(&readers[i], NULL, reader_thread, (void*)i);
    }


    sleep(WORK_TIME);

    running = 0;
    
    pthread_join(writer, NULL);
    for (int i = 0; i < READERS_COUNT; i++) {
        pthread_join(readers[i], NULL);
    }

    pthread_rwlock_destroy(&rwlock);
    return 0;
}
