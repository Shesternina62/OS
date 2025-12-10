#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define NUM_READERS 10
#define ARRAY_SIZE  50
#define MAX_WRITES 20
#define MAX_READS  40

char shared_array[ARRAY_SIZE];
int counter = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// пишущий поток
void* writer_thread(void* arg) {
    for (int i = 0; i < MAX_WRITES; i++) {
        pthread_mutex_lock(&mutex);

        snprintf(shared_array, ARRAY_SIZE, "Запись #%d", counter++);

        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
    return NULL;
}


//читающие потоки
void* reader_thread(void* arg) {
    long tid = (long)arg;

    for (int i = 0; i < MAX_READS; i++) {
        pthread_mutex_lock(&mutex);

        printf("Поток-читатель %ld: состояние массива = \"%s\"\n",
               tid, shared_array);

        pthread_mutex_unlock(&mutex);
        usleep(500000);
    }
    return NULL;
}


int main() {
    pthread_t writer;
    pthread_t readers[NUM_READERS];

    // инициализация
    strcpy(shared_array, "пусто");

    // создание пишущего потока
    pthread_create(&writer, NULL, writer_thread, NULL);

    // создание читающих потоков
    for (long i = 0; i < NUM_READERS; i++) {
        pthread_create(&readers[i], NULL, reader_thread, (void*)i);
    }

    // ожидание потоков
    pthread_join(writer, NULL);

    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    return 0;
}
