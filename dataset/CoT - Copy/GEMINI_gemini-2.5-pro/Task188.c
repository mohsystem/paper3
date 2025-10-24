#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

// To compile and run this C code, you might need to link the pthreads library:
// gcc -o your_program_name your_source_file.c -lpthread
// ./your_program_name

typedef struct {
    int n;
    sem_t foo_sem;
    sem_t bar_sem;
} FooBar;

FooBar* foobar_create(int n) {
    FooBar* obj = (FooBar*)malloc(sizeof(FooBar));
    if (obj == NULL) return NULL;
    obj->n = n;
    // Initialize foo_sem to 1 to allow the first foo() call to proceed
    sem_init(&obj->foo_sem, 0, 1);
    // Initialize bar_sem to 0 to make the first bar() call wait
    sem_init(&obj->bar_sem, 0, 0);
    return obj;
}

void print_foo() {
    printf("foo");
    fflush(stdout);
}

void print_bar() {
    printf("bar");
    fflush(stdout);
}

void* foo(void* arg) {
    FooBar* obj = (FooBar*)arg;
    for (int i = 0; i < obj->n; i++) {
        sem_wait(&obj->foo_sem);
        print_foo();
        sem_post(&obj->bar_sem);
    }
    return NULL;
}

void* bar(void* arg) {
    FooBar* obj = (FooBar*)arg;
    for (int i = 0; i < obj->n; i++) {
        sem_wait(&obj->bar_sem);
        print_bar();
        sem_post(&obj->foo_sem);
    }
    return NULL;
}

void foobar_destroy(FooBar* obj) {
    if (obj == NULL) return;
    sem_destroy(&obj->foo_sem);
    sem_destroy(&obj->bar_sem);
    free(obj);
}

void run_test(int n) {
    printf("Test Case (n = %d):\n", n);
    
    FooBar* foobar = foobar_create(n);
    if (foobar == NULL) {
        fprintf(stderr, "Failed to create FooBar object.\n");
        return;
    }
    
    pthread_t thread_a, thread_b;
    
    pthread_create(&thread_a, NULL, foo, foobar);
    pthread_create(&thread_b, NULL, bar, foobar);
    
    pthread_join(thread_a, NULL);
    pthread_join(thread_b, NULL);
    
    foobar_destroy(foobar);
    
    printf("\n\n");
}

int main() {
    run_test(1);
    run_test(2);
    run_test(5);
    run_test(10);
    run_test(100);
    
    return 0;
}