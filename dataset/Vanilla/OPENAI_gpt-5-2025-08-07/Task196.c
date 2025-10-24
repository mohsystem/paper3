#include <stdio.h>
#include <pthread.h>

typedef struct {
    int n;
    int state; // 0: zero's turn, 1: odd's turn, 2: even's turn
    int curr;
    pthread_mutex_t mutex;
    pthread_cond_t cv;
} ZeroEvenOdd;

void initZeroEvenOdd(ZeroEvenOdd* z, int n) {
    z->n = n;
    z->state = 0;
    z->curr = 1;
    pthread_mutex_init(&z->mutex, NULL);
    pthread_cond_init(&z->cv, NULL);
}

void destroyZeroEvenOdd(ZeroEvenOdd* z) {
    pthread_mutex_destroy(&z->mutex);
    pthread_cond_destroy(&z->cv);
}

void printNumberC(int x) {
    printf("%d", x);
    fflush(stdout);
}

void zero_func(ZeroEvenOdd* z, void (*printNumber)(int)) {
    for (int i = 1; i <= z->n; ++i) {
        pthread_mutex_lock(&z->mutex);
        while (z->state != 0) {
            pthread_cond_wait(&z->cv, &z->mutex);
        }
        printNumber(0);
        if (z->curr % 2 == 1) z->state = 1; else z->state = 2;
        pthread_mutex_unlock(&z->mutex);
        pthread_cond_broadcast(&z->cv);
    }
}

void even_func(ZeroEvenOdd* z, void (*printNumber)(int)) {
    for (;;) {
        pthread_mutex_lock(&z->mutex);
        while (z->state != 2 && z->curr <= z->n) {
            pthread_cond_wait(&z->cv, &z->mutex);
        }
        if (z->curr > z->n) {
            pthread_cond_broadcast(&z->cv);
            pthread_mutex_unlock(&z->mutex);
            break;
        }
        int v = z->curr;
        printNumber(v);
        z->curr++;
        z->state = 0;
        pthread_mutex_unlock(&z->mutex);
        pthread_cond_broadcast(&z->cv);
    }
}

void odd_func(ZeroEvenOdd* z, void (*printNumber)(int)) {
    for (;;) {
        pthread_mutex_lock(&z->mutex);
        while (z->state != 1 && z->curr <= z->n) {
            pthread_cond_wait(&z->cv, &z->mutex);
        }
        if (z->curr > z->n) {
            pthread_cond_broadcast(&z->cv);
            pthread_mutex_unlock(&z->mutex);
            break;
        }
        int v = z->curr;
        printNumber(v);
        z->curr++;
        z->state = 0;
        pthread_mutex_unlock(&z->mutex);
        pthread_cond_broadcast(&z->cv);
    }
}

typedef struct {
    ZeroEvenOdd* z;
    void (*printNumber)(int);
} ThreadArg;

void* thread_zero(void* arg) {
    ThreadArg* a = (ThreadArg*)arg;
    zero_func(a->z, a->printNumber);
    return NULL;
}

void* thread_even(void* arg) {
    ThreadArg* a = (ThreadArg*)arg;
    even_func(a->z, a->printNumber);
    return NULL;
}

void* thread_odd(void* arg) {
    ThreadArg* a = (ThreadArg*)arg;
    odd_func(a->z, a->printNumber);
    return NULL;
}

int main() {
    int tests[5] = {1, 2, 5, 7, 10};
    for (int ti = 0; ti < 5; ++ti) {
        int n = tests[ti];
        ZeroEvenOdd z;
        initZeroEvenOdd(&z, n);

        ThreadArg arg0 = { &z, printNumberC };
        ThreadArg arg1 = { &z, printNumberC };
        ThreadArg arg2 = { &z, printNumberC };

        pthread_t t0, t1, t2;
        pthread_create(&t0, NULL, thread_zero, &arg0);
        pthread_create(&t1, NULL, thread_even, &arg1);
        pthread_create(&t2, NULL, thread_odd, &arg2);

        pthread_join(t0, NULL);
        pthread_join(t1, NULL);
        pthread_join(t2, NULL);

        destroyZeroEvenOdd(&z);
        printf("\n");
        fflush(stdout);
    }
    return 0;
}