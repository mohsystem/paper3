#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

// Note: When compiling, link with the pthreads library, e.g., gcc -o foobar foobar.c -lpthread

typedef struct {
    int n;
    sem_t fooSem;
    sem_t barSem;
} FooBar;

FooBar* createFooBar(int n) {
    FooBar* obj = (FooBar*)malloc(sizeof(FooBar));
    if (obj == NULL) {
        perror("Failed to allocate memory for FooBar");
        return NULL;
    }
    obj->n = n;
    
    if (sem_init(&obj->fooSem, 0, 1) != 0) {
        perror("Failed to initialize fooSem");
        free(obj);
        return NULL;
    }
    if (sem_init(&obj->barSem, 0, 0) != 0) {
        perror("Failed to initialize barSem");
        sem_destroy(&obj->fooSem);
        free(obj);
        return NULL;
    }
    return obj;
}

void destroyFooBar(FooBar* obj) {
    if (obj != NULL) {
        sem_destroy(&obj->fooSem);
        sem_destroy(&obj->barSem);
        free(obj);
    }
}

void* foo(void* arg) {
    FooBar* obj = (FooBar*)arg;
    for (int i = 0; i < obj->n; i++) {
        sem_wait(&obj->fooSem);
        printf("foo");
        fflush(stdout);
        sem_post(&obj->barSem);
    }
    return NULL;
}

void* bar(void* arg) {
    FooBar* obj = (FooBar*)arg;
    for (int i = 0; i < obj->n; i++) {
        sem_wait(&obj->barSem);
        printf("bar");
        fflush(stdout);
        sem_post(&obj->fooSem);
    }
    return NULL;
}

void runTest(int n) {
    printf("Test with n = %d:\n", n);
    FooBar* fooBar = createFooBar(n);
    if (fooBar == NULL) {
        return;
    }

    pthread_t threadA, threadB;

    if (pthread_create(&threadA, NULL, foo, fooBar) != 0) {
        perror("Failed to create thread A");
        destroyFooBar(fooBar);
        return;
    }
    if (pthread_create(&threadB, NULL, bar, fooBar) != 0) {
        perror("Failed to create thread B");
        pthread_join(threadA, NULL); // Clean up thread A
        destroyFooBar(fooBar);
        return;
    }

    pthread_join(threadA, NULL);
    pthread_join(threadB, NULL);

    printf("\n\n");
    destroyFooBar(fooBar);
}

int main() {
    runTest(1);
    runTest(2);
    runTest(5);
    runTest(10);
    runTest(0); // Edge case
    return 0;
}