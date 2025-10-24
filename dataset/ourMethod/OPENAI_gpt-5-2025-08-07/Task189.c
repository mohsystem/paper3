#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef void (*print_fn)(int);

typedef struct {
    int n;
    int state; /* 0 -> zero's turn, 1 -> odd's turn, 2 -> even's turn */
    pthread_mutex_t mutex;
    pthread_cond_t cv;
} ZeroEvenOdd;

static void zeo_init(ZeroEvenOdd* z, int n) {
    if (z == NULL) { exit(1); }
    if (n < 1 || n > 1000) { fprintf(stderr, "n out of range\n"); exit(1); }
    z->n = n;
    z->state = 0;
    if (pthread_mutex_init(&z->mutex, NULL) != 0) { exit(1); }
    if (pthread_cond_init(&z->cv, NULL) != 0) { exit(1); }
}

static void zeo_destroy(ZeroEvenOdd* z) {
    if (z == NULL) return;
    pthread_mutex_destroy(&z->mutex);
    pthread_cond_destroy(&z->cv);
}

static void zeo_zero(ZeroEvenOdd* z, print_fn printNumber) {
    for (int i = 1; i <= z->n; ++i) {
        pthread_mutex_lock(&z->mutex);
        while (z->state != 0) {
            pthread_cond_wait(&z->cv, &z->mutex);
        }
        printNumber(0);
        z->state = (i % 2 == 1) ? 1 : 2;
        pthread_cond_broadcast(&z->cv);
        pthread_mutex_unlock(&z->mutex);
    }
}

static void zeo_even(ZeroEvenOdd* z, print_fn printNumber) {
    for (int i = 2; i <= z->n; i += 2) {
        pthread_mutex_lock(&z->mutex);
        while (z->state != 2) {
            pthread_cond_wait(&z->cv, &z->mutex);
        }
        printNumber(i);
        z->state = 0;
        pthread_cond_broadcast(&z->cv);
        pthread_mutex_unlock(&z->mutex);
    }
}

static void zeo_odd(ZeroEvenOdd* z, print_fn printNumber) {
    for (int i = 1; i <= z->n; i += 2) {
        pthread_mutex_lock(&z->mutex);
        while (z->state != 1) {
            pthread_cond_wait(&z->cv, &z->mutex);
        }
        printNumber(i);
        z->state = 0;
        pthread_cond_broadcast(&z->cv);
        pthread_mutex_unlock(&z->mutex);
    }
}

typedef struct {
    ZeroEvenOdd* zeo;
    print_fn printNumber;
} thread_arg;

static void* thread_zero(void* arg) {
    thread_arg* a = (thread_arg*)arg;
    zeo_zero(a->zeo, a->printNumber);
    return NULL;
}

static void* thread_even(void* arg) {
    thread_arg* a = (thread_arg*)arg;
    zeo_even(a->zeo, a->printNumber);
    return NULL;
}

static void* thread_odd(void* arg) {
    thread_arg* a = (thread_arg*)arg;
    zeo_odd(a->zeo, a->printNumber);
    return NULL;
}

static void printNumber(int x) {
    printf("%d", x);
}

static void run_test(int n) {
    ZeroEvenOdd zeo;
    zeo_init(&zeo, n);

    thread_arg a;
    a.zeo = &zeo;
    a.printNumber = printNumber;

    pthread_t t0, t1, t2;
    if (pthread_create(&t0, NULL, thread_zero, &a) != 0) exit(1);
    if (pthread_create(&t1, NULL, thread_even, &a) != 0) exit(1);
    if (pthread_create(&t2, NULL, thread_odd, &a) != 0) exit(1);

    pthread_join(t0, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    zeo_destroy(&zeo);
    printf("\n");
}

int main(void) {
    int tests[5] = {1, 2, 5, 7, 10};
    for (int i = 0; i < 5; ++i) {
        run_test(tests[i]);
    }
    return 0;
}