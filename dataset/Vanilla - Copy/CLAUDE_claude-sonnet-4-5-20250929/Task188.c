
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct {
    int n;
    sem_t foo_sem;
    sem_t bar_sem;
} Task188;

typedef struct {
    Task188* foobar;
    void (*print_func)();
} ThreadArgs;

void print_foo() {
    printf("foo");
}

void print_bar() {
    printf("bar");
}

Task188* create_task188(int n) {
    Task188* task = (Task188*)malloc(sizeof(Task188));
    task->n = n;
    sem_init(&task->foo_sem, 0, 1);
    sem_init(&task->bar_sem, 0, 0);
    return task;
}

void destroy_task188(Task188* task) {
    sem_destroy(&task->foo_sem);
    sem_destroy(&task->bar_sem);
    free(task);
}

void* foo(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    Task188* task = args->foobar;
    
    for (int i = 0; i < task->n; i++) {
        sem_wait(&task->foo_sem);
        args->print_func();
        sem_post(&task->bar_sem);
    }
    
    return NULL;
}

void* bar(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    Task188* task = args->foobar;
    
    for (int i = 0; i < task->n; i++) {
        sem_wait(&task->bar_sem);
        args->print_func();
        sem_post(&task->foo_sem);
    }
    
    return NULL;
}

void test_foobar(int n) {
    Task188* foobar = create_task188(n);
    
    ThreadArgs foo_args = {foobar, print_foo};
    ThreadArgs bar_args = {foobar, print_bar};
    
    pthread_t thread_a, thread_b;
    
    pthread_create(&thread_a, NULL, foo, &foo_args);
    pthread_create(&thread_b, NULL, bar, &bar_args);
    
    pthread_join(thread_a, NULL);
    pthread_join(thread_b, NULL);
    
    printf("\\n");
    
    destroy_task188(foobar);
}

int main() {
    printf("Test case 1: n = 1\\n");
    test_foobar(1);
    
    printf("Test case 2: n = 2\\n");
    test_foobar(2);
    
    printf("Test case 3: n = 5\\n");
    test_foobar(5);
    
    printf("Test case 4: n = 10\\n");
    test_foobar(10);
    
    printf("Test case 5: n = 3\\n");
    test_foobar(3);
    
    return 0;
}
