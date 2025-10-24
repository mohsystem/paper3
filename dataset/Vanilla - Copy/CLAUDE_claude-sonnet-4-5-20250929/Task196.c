
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

typedef struct {
    int n;
    sem_t zeroSem;
    sem_t oddSem;
    sem_t evenSem;
} Task196;

typedef struct {
    char* result;
    int index;
    pthread_mutex_t mutex;
} PrintContext;

void Task196_init(Task196* obj, int n) {
    obj->n = n;
    sem_init(&obj->zeroSem, 0, 1);
    sem_init(&obj->oddSem, 0, 0);
    sem_init(&obj->evenSem, 0, 0);
}

void Task196_destroy(Task196* obj) {
    sem_destroy(&obj->zeroSem);
    sem_destroy(&obj->oddSem);
    sem_destroy(&obj->evenSem);
}

void printNumber(int x, void* context) {
    PrintContext* ctx = (PrintContext*)context;
    pthread_mutex_lock(&ctx->mutex);
    ctx->index += sprintf(ctx->result + ctx->index, "%d", x);
    pthread_mutex_unlock(&ctx->mutex);
}

void* zero_thread(void* arg) {
    void** args = (void**)arg;
    Task196* obj = (Task196*)args[0];
    PrintContext* ctx = (PrintContext*)args[1];
    
    for (int i = 1; i <= obj->n; i++) {
        sem_wait(&obj->zeroSem);
        printNumber(0, ctx);
        if (i % 2 == 1) {
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

void testCase(int n) {
    Task196 zeo;
    Task196_init(&zeo, n);
    
    PrintContext ctx;
    ctx.result = (char*)malloc(10000);
    ctx.result[0] = '\\0';
    ctx.index = 0;
    pthread_mutex_init(&ctx.mutex, NULL);
    
    pthread_t t1, t2, t3;
    void* args[2] = {&zeo, &ctx};
    
    pthread_create(&t1, NULL, zero_thread, args);
    pthread_create(&t2, NULL, even_thread, args);
    pthread_create(&t3, NULL, odd_thread, args);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    
    printf("%s\\n", ctx.result);
    
    free(ctx.result);
    pthread_mutex_destroy(&ctx.mutex);
    Task196_destroy(&zeo);
}

int main() {
    printf("Test 1 (n=2): ");
    testCase(2);
    
    printf("Test 2 (n=5): ");
    testCase(5);
    
    printf("Test 3 (n=1): ");
    testCase(1);
    
    printf("Test 4 (n=10): ");
    testCase(10);
    
    printf("Test 5 (n=7): ");
    testCase(7);
    
    return 0;
}
