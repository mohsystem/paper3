
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
} Task196;

typedef struct {
    char* result;
    int index;
    pthread_mutex_t mutex;
} PrintContext;

Task196* Task196_create(int n) {
    Task196* obj = (Task196*)malloc(sizeof(Task196));
    obj->n = n;
    sem_init(&obj->zeroSem, 0, 1);
    sem_init(&obj->evenSem, 0, 0);
    sem_init(&obj->oddSem, 0, 0);
    return obj;
}

void Task196_destroy(Task196* obj) {
    sem_destroy(&obj->zeroSem);
    sem_destroy(&obj->evenSem);
    sem_destroy(&obj->oddSem);
    free(obj);
}

void printNumber(int num, PrintContext* ctx) {
    pthread_mutex_lock(&ctx->mutex);
    ctx->index += sprintf(ctx->result + ctx->index, "%d", num);
    pthread_mutex_unlock(&ctx->mutex);
}

void* zero_thread(void* arg) {
    void** args = (void**)arg;
    Task196* obj = (Task196*)args[0];
    PrintContext* ctx = (PrintContext*)args[1];
    
    for (int i = 0; i < obj->n; i++) {
        sem_wait(&obj->zeroSem);
        printNumber(0, ctx);
        if (i % 2 == 0) {
            sem_post(&obj->oddSem);
        } else {
            sem_post(&obj->evenSem);
        }
    }
    return NULL;
}

void* even_thread(void* arg) {
    void** args = (void**)arg;
    Task196* obj = (Task196*)args[0];
    PrintContext* ctx = (PrintContext*)args[1];
    
    for (int i = 2; i <= obj->n; i += 2) {
        sem_wait(&obj->evenSem);
        printNumber(i, ctx);
        sem_post(&obj->zeroSem);
    }
    return NULL;
}

void* odd_thread(void* arg) {
    void** args = (void**)arg;
    Task196* obj = (Task196*)args[0];
    PrintContext* ctx = (PrintContext*)args[1];
    
    for (int i = 1; i <= obj->n; i += 2) {
        sem_wait(&obj->oddSem);
        printNumber(i, ctx);
        sem_post(&obj->zeroSem);
    }
    return NULL;
}

void testCase(int n, const char* expected) {
    Task196* zeo = Task196_create(n);
    PrintContext ctx;
    ctx.result = (char*)malloc(1000);
    ctx.index = 0;
    pthread_mutex_init(&ctx.mutex, NULL);
    memset(ctx.result, 0, 1000);
    
    void* args[2] = {zeo, &ctx};
    
    pthread_t t1, t2, t3;
    pthread_create(&t1, NULL, zero_thread, args);
    pthread_create(&t2, NULL, even_thread, args);
    pthread_create(&t3, NULL, odd_thread, args);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    
    printf("Test n=%d: %s (Expected: %s, Got: %s)\\n", 
           n, strcmp(ctx.result, expected) == 0 ? "PASS" : "FAIL", 
           expected, ctx.result);
    
    pthread_mutex_destroy(&ctx.mutex);
    free(ctx.result);
    Task196_destroy(zeo);
}

int main() {
    testCase(2, "0102");
    testCase(5, "0102030405");
    testCase(1, "01");
    testCase(3, "010203");
    testCase(10, "01020304050607080910");
    return 0;
}
