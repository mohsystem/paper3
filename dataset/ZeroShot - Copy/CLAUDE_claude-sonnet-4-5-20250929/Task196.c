
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

typedef struct {
    int n;
    sem_t zeroSem;
    sem_t evenSem;
    sem_t oddSem;
} ZeroEvenOdd;

typedef struct {
    char* result;
    int index;
    pthread_mutex_t mutex;
} PrintContext;

void printNumber(int num, PrintContext* ctx) {
    pthread_mutex_lock(&ctx->mutex);
    ctx->index += sprintf(ctx->result + ctx->index, "%d", num);
    pthread_mutex_unlock(&ctx->mutex);
}

typedef struct {
    ZeroEvenOdd* zeo;
    PrintContext* ctx;
} ThreadArgs;

void* zeroThread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    ZeroEvenOdd* zeo = args->zeo;
    PrintContext* ctx = args->ctx;
    
    for (int i = 1; i <= zeo->n; i++) {
        sem_wait(&zeo->zeroSem);
        printNumber(0, ctx);
        if (i % 2 == 1) {
            sem_post(&zeo->oddSem);
        } else {
            sem_post(&zeo->evenSem);
        }
    }
    return NULL;
}

void* evenThread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    ZeroEvenOdd* zeo = args->zeo;
    PrintContext* ctx = args->ctx;
    
    for (int i = 2; i <= zeo->n; i += 2) {
        sem_wait(&zeo->evenSem);
        printNumber(i, ctx);
        sem_post(&zeo->zeroSem);
    }
    return NULL;
}

void* oddThread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    ZeroEvenOdd* zeo = args->zeo;
    PrintContext* ctx = args->ctx;
    
    for (int i = 1; i <= zeo->n; i += 2) {
        sem_wait(&zeo->oddSem);
        printNumber(i, ctx);
        sem_post(&zeo->zeroSem);
    }
    return NULL;
}

void runTest(int n) {
    ZeroEvenOdd zeo;
    zeo.n = n;
    sem_init(&zeo.zeroSem, 0, 1);
    sem_init(&zeo.evenSem, 0, 0);
    sem_init(&zeo.oddSem, 0, 0);
    
    PrintContext ctx;
    ctx.result = (char*)malloc(2000 * sizeof(char));
    ctx.index = 0;
    pthread_mutex_init(&ctx.mutex, NULL);
    
    ThreadArgs args = {&zeo, &ctx};
    
    pthread_t t1, t2, t3;
    pthread_create(&t1, NULL, zeroThread, &args);
    pthread_create(&t2, NULL, evenThread, &args);
    pthread_create(&t3, NULL, oddThread, &args);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    
    printf("n = %d: %s\\n", n, ctx.result);
    
    free(ctx.result);
    pthread_mutex_destroy(&ctx.mutex);
    sem_destroy(&zeo.zeroSem);
    sem_destroy(&zeo.evenSem);
    sem_destroy(&zeo.oddSem);
}

int main() {
    runTest(2);
    runTest(5);
    runTest(1);
    runTest(10);
    runTest(7);
    
    return 0;
}
