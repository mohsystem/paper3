
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

typedef struct {
    sem_t hydrogen_sem;
    sem_t oxygen_sem;
    pthread_mutex_t lock;
    int hydrogen_count;
    char* result;
    int result_index;
    pthread_mutex_t result_lock;
} Task187;

typedef struct {
    Task187* h2o;
    char type;
} ThreadArg;

void Task187_init(Task187* h2o, int length) {
    sem_init(&h2o->hydrogen_sem, 0, 2);
    sem_init(&h2o->oxygen_sem, 0, 0);
    pthread_mutex_init(&h2o->lock, NULL);
    pthread_mutex_init(&h2o->result_lock, NULL);
    h2o->hydrogen_count = 0;
    h2o->result = (char*)malloc((length + 1) * sizeof(char));
    h2o->result_index = 0;
}

void Task187_destroy(Task187* h2o) {
    sem_destroy(&h2o->hydrogen_sem);
    sem_destroy(&h2o->oxygen_sem);
    pthread_mutex_destroy(&h2o->lock);
    pthread_mutex_destroy(&h2o->result_lock);
    free(h2o->result);
}

void hydrogen(Task187* h2o) {
    sem_wait(&h2o->hydrogen_sem);
    pthread_mutex_lock(&h2o->lock);
    
    pthread_mutex_lock(&h2o->result_lock);
    h2o->result[h2o->result_index++] = 'H';
    pthread_mutex_unlock(&h2o->result_lock);
    
    h2o->hydrogen_count++;
    if (h2o->hydrogen_count == 2) {
        sem_post(&h2o->oxygen_sem);
    }
    pthread_mutex_unlock(&h2o->lock);
}

void oxygen(Task187* h2o) {
    sem_wait(&h2o->oxygen_sem);
    pthread_mutex_lock(&h2o->lock);
    
    pthread_mutex_lock(&h2o->result_lock);
    h2o->result[h2o->result_index++] = 'O';
    pthread_mutex_unlock(&h2o->result_lock);
    
    h2o->hydrogen_count = 0;
    sem_post(&h2o->hydrogen_sem);
    sem_post(&h2o->hydrogen_sem);
    pthread_mutex_unlock(&h2o->lock);
}

void* thread_func(void* arg) {
    ThreadArg* targ = (ThreadArg*)arg;
    if (targ->type == 'H') {
        hydrogen(targ->h2o);
    } else {
        oxygen(targ->h2o);
    }
    return NULL;
}

char* buildWater(const char* water) {
    int length = strlen(water);
    Task187 h2o;
    Task187_init(&h2o, length);
    
    pthread_t* threads = (pthread_t*)malloc(length * sizeof(pthread_t));
    ThreadArg* args = (ThreadArg*)malloc(length * sizeof(ThreadArg));
    
    for (int i = 0; i < length; i++) {
        args[i].h2o = &h2o;
        args[i].type = water[i];
        pthread_create(&threads[i], NULL, thread_func, &args[i]);
    }
    
    for (int i = 0; i < length; i++) {
        pthread_join(threads[i], NULL);
    }
    
    h2o.result[length] = '\\0';
    char* result = strdup(h2o.result);
    
    free(threads);
    free(args);
    Task187_destroy(&h2o);
    
    return result;
}

int main() {
    const char* testCases[] = {"HOH", "OOHHHH", "HHOHHO", "HHHHHHOOO", "OHHHOH"};
    int numTests = 5;
    
    for (int i = 0; i < numTests; i++) {
        char* output = buildWater(testCases[i]);
        printf("Input: %s -> Output: %s\\n", testCases[i], output);
        free(output);
    }
    
    return 0;
}
