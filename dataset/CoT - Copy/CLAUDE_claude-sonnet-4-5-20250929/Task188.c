
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct {
    int n;
    sem_t foo_sem;
    sem_t bar_sem;
} Task188;

typedef struct {
    Task188* foobar;
    void (*printFunc)();
} ThreadArgs;

void print_foo() {
    printf("foo");
}

void print_bar() {
    printf("bar");
}

void* foo_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    Task188* foobar = args->foobar;
    
    for (int i = 0; i < foobar->n; i++) {
        sem_wait(&foobar->foo_sem);
        args->printFunc();
        sem_post(&foobar->bar_sem);
    }
    return NULL;
}

void* bar_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    Task188* foobar = args->foobar;
    
    for (int i = 0; i < foobar->n; i++) {
        sem_wait(&foobar->bar_sem);
        args->printFunc();
        sem_post(&foobar->foo_sem);
    }
    return NULL;
}

void init_task188(Task188* foobar, int n) {
    foobar->n = n;
    sem_init(&foobar->foo_sem, 0, 1);
    sem_init(&foobar->bar_sem, 0, 0);
}

void destroy_task188(Task188* foobar) {
    sem_destroy(&foobar->foo_sem);
    sem_destroy(&foobar->bar_sem);
}

void test_foobar(int n) {
    Task188 foobar;
    init_task188(&foobar, n);
    
    pthread_t thread_a, thread_b;
    
    ThreadArgs args_a = {&foobar, print_foo};
    ThreadArgs args_b = {&foobar, print_bar};
    
    pthread_create(&thread_a, NULL, foo_thread, &args_a);
    pthread_create(&thread_b, NULL, bar_thread, &args_b);
    
    pthread_join(thread_a, NULL);
    pthread_join(thread_b, NULL);
    
    printf("\\n");
    destroy_task188(&foobar);
}

int main() {
    printf("Test case 1:\\n");
    test_foobar(1);
    
    printf("\\nTest case 2:\\n");
    test_foobar(2);
    
    printf("\\nTest case 3:\\n");
    test_foobar(5);
    
    printf("\\nTest case 4:\\n");
    test_foobar(10);
    
    printf("\\nTest case 5:\\n");
    test_foobar(3);
    
    return 0;
}
