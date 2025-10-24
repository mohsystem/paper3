#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

// Prints number to console
void printNumber(int x) {
    printf("%d", x);
    fflush(stdout);
}

typedef struct {
    int n;
    int current;
    int state; // 0: zero, 1: odd, 2: even
    pthread_mutex_t mtx;
    pthread_cond_t cv;
} ZeroEvenOdd;

void ZeroEvenOdd_init(ZeroEvenOdd* z, int n) {
    z->n = n;
    z->current = 1;
    z->state = 0;
    pthread_mutex_init(&z->mtx, NULL);
    pthread_cond_init(&z->cv, NULL);
}

void ZeroEvenOdd_destroy(ZeroEvenOdd* z) {
    pthread_mutex_destroy(&z->mtx);
    pthread_cond_destroy(&z->cv);
}

typedef struct {
    char* data;
    size_t size;
    size_t cap;
    pthread_mutex_t mtx;
} OutputBuffer;

void outbuf_init(OutputBuffer* ob) {
    ob->cap = 64;
    ob->size = 0;
    ob->data = (char*)malloc(ob->cap);
    ob->data[0] = '\0';
    pthread_mutex_init(&ob->mtx, NULL);
}

void outbuf_append_str(OutputBuffer* ob, const char* s) {
    size_t len = strlen(s);
    pthread_mutex_lock(&ob->mtx);
    if (ob->size + len + 1 > ob->cap) {
        while (ob->size + len + 1 > ob->cap) ob->cap *= 2;
        ob->data = (char*)realloc(ob->data, ob->cap);
    }
    memcpy(ob->data + ob->size, s, len + 1);
    ob->size += len;
    pthread_mutex_unlock(&ob->mtx);
}

void outbuf_destroy(OutputBuffer* ob) {
    pthread_mutex_destroy(&ob->mtx);
    // caller may keep ob->data
}

static OutputBuffer* g_outbuf = NULL;

void printNumber_capture(int x) {
    // console print
    printNumber(x);
    // append to buffer
    if (g_outbuf) {
        char tmp[16];
        snprintf(tmp, sizeof(tmp), "%d", x);
        outbuf_append_str(g_outbuf, tmp);
    }
}

void ZeroEvenOdd_zero(ZeroEvenOdd* z, void (*printNumberFn)(int)) {
    for (int i = 1; i <= z->n; ++i) {
        pthread_mutex_lock(&z->mtx);
        while (z->state != 0) {
            pthread_cond_wait(&z->cv, &z->mtx);
        }
        pthread_mutex_unlock(&z->mtx);

        printNumberFn(0);

        pthread_mutex_lock(&z->mtx);
        if ((z->current & 1) == 1) z->state = 1;
        else z->state = 2;
        pthread_cond_broadcast(&z->cv);
        pthread_mutex_unlock(&z->mtx);
    }
}

void ZeroEvenOdd_odd(ZeroEvenOdd* z, void (*printNumberFn)(int)) {
    while (1) {
        pthread_mutex_lock(&z->mtx);
        while (z->state != 1 && z->current <= z->n) {
            pthread_cond_wait(&z->cv, &z->mtx);
        }
        if (z->current > z->n) {
            pthread_cond_broadcast(&z->cv);
            pthread_mutex_unlock(&z->mtx);
            return;
        }
        int val = z->current;
        pthread_mutex_unlock(&z->mtx);

        printNumberFn(val);

        pthread_mutex_lock(&z->mtx);
        z->current++;
        z->state = 0;
        pthread_cond_broadcast(&z->cv);
        pthread_mutex_unlock(&z->mtx);
    }
}

void ZeroEvenOdd_even(ZeroEvenOdd* z, void (*printNumberFn)(int)) {
    while (1) {
        pthread_mutex_lock(&z->mtx);
        while (z->state != 2 && z->current <= z->n) {
            pthread_cond_wait(&z->cv, &z->mtx);
        }
        if (z->current > z->n) {
            pthread_cond_broadcast(&z->cv);
            pthread_mutex_unlock(&z->mtx);
            return;
        }
        int val = z->current;
        pthread_mutex_unlock(&z->mtx);

        printNumberFn(val);

        pthread_mutex_lock(&z->mtx);
        z->current++;
        z->state = 0;
        pthread_cond_broadcast(&z->cv);
        pthread_mutex_unlock(&z->mtx);
    }
}

typedef struct {
    ZeroEvenOdd* z;
    void (*printNumberFn)(int);
} ThreadArgs;

void* zero_thread(void* arg) {
    ThreadArgs* a = (ThreadArgs*)arg;
    ZeroEvenOdd_zero(a->z, a->printNumberFn);
    return NULL;
}

void* odd_thread(void* arg) {
    ThreadArgs* a = (ThreadArgs*)arg;
    ZeroEvenOdd_odd(a->z, a->printNumberFn);
    return NULL;
}

void* even_thread(void* arg) {
    ThreadArgs* a = (ThreadArgs*)arg;
    ZeroEvenOdd_even(a->z, a->printNumberFn);
    return NULL;
}

char* runZeroEvenOdd(int n) {
    ZeroEvenOdd z;
    ZeroEvenOdd_init(&z, n);

    OutputBuffer ob;
    outbuf_init(&ob);
    g_outbuf = &ob;

    pthread_t t0, t1, t2;
    ThreadArgs args;
    args.z = &z;
    args.printNumberFn = printNumber_capture;

    pthread_create(&t0, NULL, zero_thread, &args);
    pthread_create(&t1, NULL, even_thread, &args);
    pthread_create(&t2, NULL, odd_thread, &args);

    pthread_join(t0, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    ZeroEvenOdd_destroy(&z);
    g_outbuf = NULL;

    // Return the buffer data; caller owns it
    // Do not free ob.data here
    return ob.data;
}

int main() {
    int tests[5] = {1, 2, 5, 6, 7};
    for (int i = 0; i < 5; ++i) {
        char* s = runZeroEvenOdd(tests[i]);
        printf("\n");
        // Optionally use s; not printing it again to avoid duplicate output
        free(s);
    }
    return 0;
}