
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
    int current;
} Task189;

typedef struct {
    char* result;
    int pos;
} PrintContext;

void printNumber(int x, PrintContext* ctx) {
    int len = snprintf(NULL, 0, "%d", x);
    snprintf(ctx->result + ctx->pos, len + 1, "%d", x);
    ctx->pos += len;
}

void Task189_init(Task189* obj, int n) {
    obj->n = n;
    obj->current = 1;
    sem_init(&obj->zeroSem, 0, 1);
    sem_init(&obj->evenSem, 0, 0);
    sem_init(&obj->oddSem, 0, 0);
}

void Task189_destroy(Task189* obj) {
    sem_destroy(&obj->zeroSem);
    sem_destroy(&obj->evenSem);
    sem_destroy(&obj->oddSem);
}

typedef struct {
    Task189* obj;
    PrintContext* ctx;
} ThreadArgs;

void* zero_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    Task189* obj = args->obj;
    PrintContext* ctx = args->ctx;
    
    for (int i = 0; i < obj->n; i++) {
        sem_wait(&obj->zeroSem);
        printNumber(0, ctx);
        if (obj->current % 2 == 1) {
            sem_post(&obj->oddSem);
        } else {
            sem_post(&obj->evenSem);
        }
    }
    return NULL;
}

void* even_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    Task189* obj = args->obj;
    PrintContext* ctx = args->ctx;
    
    for (int i = 2; i <= obj->n; i += 2) {
        sem_wait(&obj->evenSem);
        printNumber(i, ctx);
        obj->current++;
        sem_post(&obj->zeroSem);
    }
    return NULL;
}

void* odd_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    Task189* obj = args->obj;
    PrintContext* ctx = args->ctx;
    
    for (int i = 1; i <= obj->n; i += 2) {
        sem_wait(&obj->oddSem);
        printNumber(i, ctx);
        obj->current++;
        sem_post(&obj->zeroSem);
    }
    return NULL;
}

void testCase(int n, const char* expected) {
    Task189 zeo;
    Task189_init(&zeo, n);
    
    char result[1000] = {0};
    PrintContext ctx = {result, 0};
    
    ThreadArgs args = {&zeo, &ctx};
    
    pthread_t t1, t2, t3;
    pthread_create(&t1, NULL, zero_thread, &args);
    pthread_create(&t2, NULL, even_thread, &args);
    pthread_create(&t3, NULL, odd_thread, &args);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    
    printf("Input: n = %d\\n", n);
    printf("Output: \\"%s\\"\\n", result);
    printf("Expected: \\"%s\\"\\n", expected);
    printf("Pass: %s\\n", strcmp(result, expected) == 0 ? "true" : "false");
    printf("\\n");
    
    Task189_destroy(&zeo);
}

int main() {
    testCase(2, "0102");
    testCase(5, "0102030405");
    testCase(1, "01");
    testCase(3, "010203");
    testCase(10, "01020304050607080910");
    
    return 0;
}
