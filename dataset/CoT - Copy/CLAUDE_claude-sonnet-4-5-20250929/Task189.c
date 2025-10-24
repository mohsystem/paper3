
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    int n;
    sem_t zeroSem;
    sem_t evenSem;
    sem_t oddSem;
} Task189;

typedef struct {
    char* result;
    int index;
    pthread_mutex_t mutex;
} PrintContext;

void Task189_init(Task189* obj, int n) {
    obj->n = n;
    sem_init(&obj->zeroSem, 0, 1);
    sem_init(&obj->evenSem, 0, 0);
    sem_init(&obj->oddSem, 0, 0);
}

void Task189_destroy(Task189* obj) {
    sem_destroy(&obj->zeroSem);
    sem_destroy(&obj->evenSem);
    sem_destroy(&obj->oddSem);
}

void printNumber(int num, PrintContext* ctx) {
    pthread_mutex_lock(&ctx->mutex);
    ctx->index += sprintf(ctx->result + ctx->index, "%d", num);
    pthread_mutex_unlock(&ctx->mutex);
}

typedef struct {
    Task189* obj;
    PrintContext* ctx;
} ThreadArgs;

void* zero_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    for (int i = 0; i < args->obj->n; i++) {
        sem_wait(&args->obj->zeroSem);
        printNumber(0, args->ctx);
        if (i % 2 == 0) {
            sem_post(&args->obj->oddSem);
        } else {
            sem_post(&args->obj->evenSem);
        }
    }
    return NULL;
}

void* even_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    for (int i = 2; i <= args->obj->n; i += 2) {
        sem_wait(&args->obj->evenSem);
        printNumber(i, args->ctx);
        sem_post(&args->obj->zeroSem);
    }
    return NULL;
}

void* odd_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    for (int i = 1; i <= args->obj->n; i += 2) {
        sem_wait(&args->obj->oddSem);
        printNumber(i, args->ctx);
        sem_post(&args->obj->zeroSem);
    }
    return NULL;
}

void testCase(int n, const char* expected) {
    Task189 zeroEvenOdd;
    Task189_init(&zeroEvenOdd, n);
    
    PrintContext ctx;
    ctx.result = (char*)malloc(2 * n + 1);
    ctx.result[0] = '\\0';
    ctx.index = 0;
    pthread_mutex_init(&ctx.mutex, NULL);
    
    ThreadArgs args = {&zeroEvenOdd, &ctx};
    
    pthread_t threadA, threadB, threadC;
    pthread_create(&threadA, NULL, zero_thread, &args);
    pthread_create(&threadB, NULL, even_thread, &args);
    pthread_create(&threadC, NULL, odd_thread, &args);
    
    pthread_join(threadA, NULL);
    pthread_join(threadB, NULL);
    pthread_join(threadC, NULL);
    
    printf("Input: n = %d\\n", n);
    printf("Expected: %s\\n", expected);
    printf("Output: %s\\n", ctx.result);
    printf("Test %s\\n\\n", strcmp(ctx.result, expected) == 0 ? "PASSED" : "FAILED");
    
    free(ctx.result);
    pthread_mutex_destroy(&ctx.mutex);
    Task189_destroy(&zeroEvenOdd);
}

int main() {
    testCase(2, "0102");
    testCase(5, "0102030405");
    testCase(1, "01");
    testCase(3, "010203");
    testCase(4, "01020304");
    return 0;
}
