#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Note: When compiling, link with the pthreads library.
// Example: gcc -pthread your_file.c -o your_program

typedef struct {
    int n;
    pthread_mutex_t mtx;
    pthread_cond_t cond_foo;
    pthread_cond_t cond_bar;
    int turn_flag; // 0 for foo, 1 for bar
} FooBar;

FooBar* foobar_create(int n) {
    FooBar* obj = (FooBar*)malloc(sizeof(FooBar));
    obj->n = n;
    pthread_mutex_init(&obj->mtx, NULL);
    pthread_cond_init(&obj->cond_foo, NULL);
    pthread_cond_init(&obj->cond_bar, NULL);
    obj->turn_flag = 0; // Foo's turn starts
    return obj;
}

void foobar_destroy(FooBar* obj) {
    pthread_mutex_destroy(&obj->mtx);
    pthread_cond_destroy(&obj->cond_foo);
    pthread_cond_destroy(&obj->cond_bar);
    free(obj);
}

void print_foo() {
    printf("foo");
}

void print_bar() {
    printf("bar");
}

void* foo_thread_func(void* arg) {
    FooBar* obj = (FooBar*)arg;
    for (int i = 0; i < obj->n; i++) {
        pthread_mutex_lock(&obj->mtx);
        while (obj->turn_flag != 0) {
            pthread_cond_wait(&obj->cond_foo, &obj->mtx);
        }
        print_foo();
        obj->turn_flag = 1;
        pthread_cond_signal(&obj->cond_bar);
        pthread_mutex_unlock(&obj->mtx);
    }
    return NULL;
}

void* bar_thread_func(void* arg) {
    FooBar* obj = (FooBar*)arg;
    for (int i = 0; i < obj->n; i++) {
        pthread_mutex_lock(&obj->mtx);
        while (obj->turn_flag != 1) {
            pthread_cond_wait(&obj->cond_bar, &obj->mtx);
        }
        print_bar();
        obj->turn_flag = 0;
        pthread_cond_signal(&obj->cond_foo);
        pthread_mutex_unlock(&obj->mtx);
    }
    return NULL;
}

void runTest(int n) {
    if (n <= 0) return;
    FooBar* foobar = foobar_create(n);
    pthread_t threadA, threadB;

    pthread_create(&threadA, NULL, foo_thread_func, foobar);
    pthread_create(&threadB, NULL, bar_thread_func, foobar);

    pthread_join(threadA, NULL);
    pthread_join(threadB, NULL);

    foobar_destroy(foobar);
}

int main() {
    int testCases[] = {1, 2, 5, 10, 100};
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);
    for (int i = 0; i < num_tests; i++) {
        int n = testCases[i];
        runTest(n);
        printf("\n");
    }
    return 0;
}