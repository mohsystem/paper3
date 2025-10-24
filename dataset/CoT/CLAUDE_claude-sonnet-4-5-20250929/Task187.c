
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

typedef struct {
    sem_t hydrogenSemaphore;
    sem_t oxygenSemaphore;
    pthread_mutex_t lock;
    int hydrogenCount;
} Task187;

typedef struct {
    Task187* h2o;
    char* result;
    int* resultIndex;
    pthread_mutex_t* resultLock;
    char type;
} ThreadData;

void Task187_init(Task187* h2o) {
    sem_init(&h2o->hydrogenSemaphore, 0, 2);
    sem_init(&h2o->oxygenSemaphore, 0, 0);
    pthread_mutex_init(&h2o->lock, NULL);
    h2o->hydrogenCount = 0;
}

void Task187_destroy(Task187* h2o) {
    sem_destroy(&h2o->hydrogenSemaphore);
    sem_destroy(&h2o->oxygenSemaphore);
    pthread_mutex_destroy(&h2o->lock);
}

void hydrogen(Task187* h2o, char* result, int* resultIndex, pthread_mutex_t* resultLock) {
    sem_wait(&h2o->hydrogenSemaphore);
    pthread_mutex_lock(&h2o->lock);
    
    pthread_mutex_lock(resultLock);
    result[(*resultIndex)++] = 'H';
    pthread_mutex_unlock(resultLock);
    
    h2o->hydrogenCount++;
    if (h2o->hydrogenCount == 2) {
        sem_post(&h2o->oxygenSemaphore);
        h2o->hydrogenCount = 0;
    }
    pthread_mutex_unlock(&h2o->lock);
}

void oxygen(Task187* h2o, char* result, int* resultIndex, pthread_mutex_t* resultLock) {
    sem_wait(&h2o->oxygenSemaphore);
    pthread_mutex_lock(&h2o->lock);
    
    pthread_mutex_lock(resultLock);
    result[(*resultIndex)++] = 'O';
    pthread_mutex_unlock(resultLock);
    
    sem_post(&h2o->hydrogenSemaphore);
    sem_post(&h2o->hydrogenSemaphore);
    pthread_mutex_unlock(&h2o->lock);
}

void* threadFunction(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    if (data->type == 'H') {
        hydrogen(data->h2o, data->result, data->resultIndex, data->resultLock);
    } else {
        oxygen(data->h2o, data->result, data->resultIndex, data->resultLock);
    }
    return NULL;
}

void testCase(const char* water) {
    Task187 h2o;
    Task187_init(&h2o);
    
    int len = strlen(water);
    char* result = (char*)malloc((len + 1) * sizeof(char));
    int resultIndex = 0;
    pthread_mutex_t resultLock;
    pthread_mutex_init(&resultLock, NULL);
    
    pthread_t* threads = (pthread_t*)malloc(len * sizeof(pthread_t));
    ThreadData* threadData = (ThreadData*)malloc(len * sizeof(ThreadData));
    
    for (int i = 0; i < len; i++) {
        threadData[i].h2o = &h2o;
        threadData[i].result = result;
        threadData[i].resultIndex = &resultIndex;
        threadData[i].resultLock = &resultLock;
        threadData[i].type = water[i];
        pthread_create(&threads[i], NULL, threadFunction, &threadData[i]);
    }
    
    for (int i = 0; i < len; i++) {
        pthread_join(threads[i], NULL);
    }
    
    result[len] = '\\0';
    printf("Input: %s -> Output: %s\\n", water, result);
    
    free(result);
    free(threads);
    free(threadData);
    pthread_mutex_destroy(&resultLock);
    Task187_destroy(&h2o);
}

int main() {
    testCase("HOH");
    testCase("OOHHHH");
    testCase("HHHHHHOOO");
    testCase("HHHOHHHOOO");
    testCase("HHOOHH");
    return 0;
}
