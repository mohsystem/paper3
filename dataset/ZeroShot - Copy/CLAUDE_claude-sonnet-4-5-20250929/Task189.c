
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

void initZeroEvenOdd(ZeroEvenOdd* obj, int n) {
    obj->n = n;
    sem_init(&obj->zeroSem, 0, 1);
    sem_init(&obj->evenSem, 0, 0);
    sem_init(&obj->oddSem, 0, 0);
}

void destroyZeroEvenOdd(ZeroEvenOdd* obj) {
    sem_destroy(&obj->zeroSem);
    sem_destroy(&obj->evenSem);
    sem_destroy(&obj->oddSem);
}

void printNumber(int num, void* context) {
    PrintContext* ctx = (PrintContext*)context;
    pthread_mutex_lock(&ctx->mutex);
    int len = snprintf(NULL, 0, "%d", num);
    snprintf(ctx->result + ctx->index, len + 1, "%d", num);
    ctx->index += len;
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
    initZeroEvenOdd(&zeo, n);
    
    PrintContext ctx;
    ctx.result = (char*)malloc(1000);
    ctx.result[0] = '\\0';
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
    
    printf("%s\\n", ctx.result);
    
    free(ctx.result);
    pthread_mutex_destroy(&ctx.mutex);
    destroyZeroEvenOdd(&zeo);
}

int main() {
    printf("Test 1 (n=2): ");
    runTest(2);
    
    printf("Test 2 (n=5): ");
    runTest(5);
    
    printf("Test 3 (n=1): ");
    runTest(1);
    
    printf("Test 4 (n=10): ");
    runTest(10);
    
    printf("Test 5 (n=7): ");
    runTest(7);
    
    return 0;
}
