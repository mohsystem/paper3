
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct {
    int n;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int fooTurn;
} Task188;

typedef struct {
    Task188* fooBar;
    void (*printFunc)(void);
} ThreadArgs;

void printFoo() {
    printf("foo");
}

void printBar() {
    printf("bar");
}

Task188* createTask188(int n) {
    Task188* obj = (Task188*)malloc(sizeof(Task188));
    obj->n = n;
    obj->fooTurn = 1;
    pthread_mutex_init(&obj->mutex, NULL);
    pthread_cond_init(&obj->cond, NULL);
    return obj;
}

void* foo(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    Task188* obj = args->fooBar;
    
    for (int i = 0; i < obj->n; i++) {
        pthread_mutex_lock(&obj->mutex);
        while (!obj->fooTurn) {
            pthread_cond_wait(&obj->cond, &obj->mutex);
        }
        args->printFunc();
        obj->fooTurn = 0;
        pthread_cond_signal(&obj->cond);
        pthread_mutex_unlock(&obj->mutex);
    }
    return NULL;
}

void* bar(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    Task188* obj = args->fooBar;
    
    for (int i = 0; i < obj->n; i++) {
        pthread_mutex_lock(&obj->mutex);
        while (obj->fooTurn) {
            pthread_cond_wait(&obj->cond, &obj->mutex);
        }
        args->printFunc();
        obj->fooTurn = 1;
        pthread_cond_signal(&obj->cond);
        pthread_mutex_unlock(&obj->mutex);
    }
    return NULL;
}

void testFooBar(int n) {
    Task188* fooBar = createTask188(n);
    
    pthread_t threadA, threadB;
    ThreadArgs argsA = {fooBar, printFoo};
    ThreadArgs argsB = {fooBar, printBar};
    
    pthread_create(&threadA, NULL, foo, &argsA);
    pthread_create(&threadB, NULL, bar, &argsB);
    
    pthread_join(threadA, NULL);
    pthread_join(threadB, NULL);
    
    pthread_mutex_destroy(&fooBar->mutex);
    pthread_cond_destroy(&fooBar->cond);
    free(fooBar);
    
    printf("\\n");
}

int main() {
    // Test case 1
    printf("Test case 1: n = 1\\n");
    testFooBar(1);
    
    // Test case 2
    printf("Test case 2: n = 2\\n");
    testFooBar(2);
    
    // Test case 3
    printf("Test case 3: n = 5\\n");
    testFooBar(5);
    
    // Test case 4
    printf("Test case 4: n = 10\\n");
    testFooBar(10);
    
    // Test case 5
    printf("Test case 5: n = 3\\n");
    testFooBar(3);
    
    return 0;
}
