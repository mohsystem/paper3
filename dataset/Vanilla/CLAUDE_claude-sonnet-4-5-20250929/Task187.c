
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct {
    sem_t hydrogenSemaphore;
    sem_t oxygenSemaphore;
    pthread_mutex_t mutex;
    int hydrogenCount;
} Task187;

typedef struct {
    Task187* h2o;
    char* result;
    int* resultIndex;
    pthread_mutex_t* resultMutex;
    char type;
} ThreadData;

void Task187_init(Task187* h2o) {
    sem_init(&h2o->hydrogenSemaphore, 0, 2);
    sem_init(&h2o->oxygenSemaphore, 0, 0);
    pthread_mutex_init(&h2o->mutex, NULL);
    h2o->hydrogenCount = 0;
}

void Task187_destroy(Task187* h2o) {
    sem_destroy(&h2o->hydrogenSemaphore);
    sem_destroy(&h2o->oxygenSemaphore);
    pthread_mutex_destroy(&h2o->mutex);
}

void releaseHydrogen(char* result, int* resultIndex, pthread_mutex_t* resultMutex) {
    pthread_mutex_lock(resultMutex);
    result[(*resultIndex)++] = 'H';
    pthread_mutex_unlock(resultMutex);
}

void releaseOxygen(char* result, int* resultIndex, pthread_mutex_t* resultMutex) {
    pthread_mutex_lock(resultMutex);
    result[(*resultIndex)++] = 'O';
    pthread_mutex_unlock(resultMutex);
}

void hydrogen(Task187* h2o, char* result, int* resultIndex, pthread_mutex_t* resultMutex) {
    sem_wait(&h2o->hydrogenSemaphore);
    
    pthread_mutex_lock(&h2o->mutex);
    h2o->hydrogenCount++;
    if (h2o->hydrogenCount == 2) {
        sem_post(&h2o->oxygenSemaphore);
    }
    pthread_mutex_unlock(&h2o->mutex);
    
    releaseHydrogen(result, resultIndex, resultMutex);
}

void oxygen(Task187* h2o, char* result, int* resultIndex, pthread_mutex_t* resultMutex) {
    sem_wait(&h2o->oxygenSemaphore);
    
    releaseOxygen(result, resultIndex, resultMutex);
    
    pthread_mutex_lock(&h2o->mutex);
    h2o->hydrogenCount = 0;
    pthread_mutex_unlock(&h2o->mutex);
    
    sem_post(&h2o->hydrogenSemaphore);
    sem_post(&h2o->hydrogenSemaphore);
}

void* threadFunc(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    if (data->type == 'H') {
        hydrogen(data->h2o, data->result, data->resultIndex, data->resultMutex);
    } else {
        oxygen(data->h2o, data->result, data->resultIndex, data->resultMutex);
    }
    return NULL;
}

int main() {
    char* testCases[] = {"HOH", "OOHHHH", "HHOHHO", "HHHHHHOOO", "OHHHOH"};
    int numTests = 5;
    
    for (int i = 0; i < numTests; i++) {
        printf("Input: %s\\n", testCases[i]);
        
        Task187 h2o;
        Task187_init(&h2o);
        
        int len = strlen(testCases[i]);
        char* result = (char*)malloc((len + 1) * sizeof(char));
        int resultIndex = 0;
        pthread_mutex_t resultMutex;
        pthread_mutex_init(&resultMutex, NULL);
        
        pthread_t* threads = (pthread_t*)malloc(len * sizeof(pthread_t));
        ThreadData* threadData = (ThreadData*)malloc(len * sizeof(ThreadData));
        
        for (int j = 0; j < len; j++) {
            threadData[j].h2o = &h2o;
            threadData[j].result = result;
            threadData[j].resultIndex = &resultIndex;
            threadData[j].resultMutex = &resultMutex;
            threadData[j].type = testCases[i][j];
            pthread_create(&threads[j], NULL, threadFunc, &threadData[j]);
        }
        
        for (int j = 0; j < len; j++) {
            pthread_join(threads[j], NULL);
        }
        
        result[len] = '\\0';
        printf("Output: %s\\n\\n", result);
        
        free(result);
        free(threads);
        free(threadData);
        pthread_mutex_destroy(&resultMutex);
        Task187_destroy(&h2o);
    }
    
    return 0;
}
