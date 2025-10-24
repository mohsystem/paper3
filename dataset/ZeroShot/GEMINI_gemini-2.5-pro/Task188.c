#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

// Struct to hold shared data for threads
typedef struct {
    int n;
    sem_t foo_sem;
    sem_t bar_sem;
} FooBar;

// Thread function to print "foo"
void* foo(void* arg) {
    FooBar* foobar = (FooBar*)arg;
    for (int i = 0; i < foobar->n; i++) {
        sem_wait(&foobar->foo_sem);
        printf("foo");
        fflush(stdout); // Ensure "foo" is printed before "bar" is signaled
        sem_post(&foobar->bar_sem);
    }
    return NULL;
}

// Thread function to print "bar"
void* bar(void* arg) {
    FooBar* foobar = (FooBar*)arg;
    for (int i = 0; i < foobar->n; i++) {
        sem_wait(&foobar->bar_sem);
        printf("bar");
        fflush(stdout); // Ensure "bar" is printed before "foo" is signaled
        sem_post(&foobar->foo_sem);
    }
    return NULL;
}

// Helper to create and run a test case
void run_test(int n) {
    printf("Running test for n = %d\n", n);
    if (n <= 0) {
        printf("Output: \"\"\n");
        printf("--------------------\n");
        return;
    }

    FooBar* foobar = (FooBar*)malloc(sizeof(FooBar));
    if (foobar == NULL) {
        perror("Failed to allocate memory for FooBar");
        return;
    }
    foobar->n = n;

    // Initialize semaphores
    // sem_init(sem_t *sem, int pshared, unsigned int value);
    // pshared = 0 -> semaphore is shared between threads of the same process
    sem_init(&foobar->foo_sem, 0, 1); // foo starts unlocked
    sem_init(&foobar->bar_sem, 0, 0); // bar starts locked

    pthread_t threadA, threadB;

    printf("Output: \"");
    fflush(stdout);

    // Create threads
    pthread_create(&threadA, NULL, foo, foobar);
    pthread_create(&threadB, NULL, bar, foobar);

    // Wait for threads to finish
    pthread_join(threadA, NULL);
    pthread_join(threadB, NULL);
    
    printf("\"\n");

    // Clean up resources
    sem_destroy(&foobar->foo_sem);
    sem_destroy(&foobar->bar_sem);
    free(foobar);
    
    printf("--------------------\n");
}

int main() {
    int test_cases[] = {1, 2, 5, 10, 0};
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_tests; i++) {
        run_test(test_cases[i]);
    }

    return 0;
}