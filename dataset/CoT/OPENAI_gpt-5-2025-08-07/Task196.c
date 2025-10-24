#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct {
    int n;
    int curr; // next number to print (1..n)
    int turn; // 0 -> zero's turn, 1 -> number's turn
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    char* buf;
    size_t cap;
    size_t pos;
} ZeroEvenOdd;

static void append_to_buffer(ZeroEvenOdd* z, const char* s) {
    size_t len = strlen(s);
    if (z->pos + len >= z->cap) {
        // Prevent overflow; truncate safely
        len = (z->cap > z->pos) ? (z->cap - z->pos - 1) : 0;
    }
    if (len > 0) {
        memcpy(z->buf + z->pos, s, len);
        z->pos += len;
        z->buf[z->pos] = '\0';
    }
}

static void printNumber(ZeroEvenOdd* z, int x) {
    // Print to console
    printf("%d", x);
    // Append to buffer
    char tmp[16];
    snprintf(tmp, sizeof(tmp), "%d", x);
    append_to_buffer(z, tmp);
}

static void* zero_thread(void* arg) {
    ZeroEvenOdd* z = (ZeroEvenOdd*)arg;
    for (int i = 1; i <= z->n; ++i) {
        pthread_mutex_lock(&z->mutex);
        while (z->turn != 0) {
            pthread_cond_wait(&z->cond, &z->mutex);
        }
        printNumber(z, 0);
        z->turn = 1;
        pthread_mutex_unlock(&z->mutex);
        pthread_cond_broadcast(&z->cond);
    }
    return NULL;
}

static void* even_thread(void* arg) {
    ZeroEvenOdd* z = (ZeroEvenOdd*)arg;
    for (;;) {
        pthread_mutex_lock(&z->mutex);
        while (!((z->turn == 1) && (z->curr % 2 == 0)) && z->curr <= z->n) {
            pthread_cond_wait(&z->cond, &z->mutex);
        }
        if (z->curr > z->n) {
            pthread_mutex_unlock(&z->mutex);
            pthread_cond_broadcast(&z->cond);
            return NULL;
        }
        printNumber(z, z->curr);
        z->curr++;
        z->turn = 0;
        pthread_mutex_unlock(&z->mutex);
        pthread_cond_broadcast(&z->cond);
    }
    return NULL;
}

static void* odd_thread(void* arg) {
    ZeroEvenOdd* z = (ZeroEvenOdd*)arg;
    for (;;) {
        pthread_mutex_lock(&z->mutex);
        while (!((z->turn == 1) && (z->curr % 2 == 1)) && z->curr <= z->n) {
            pthread_cond_wait(&z->cond, &z->mutex);
        }
        if (z->curr > z->n) {
            pthread_mutex_unlock(&z->mutex);
            pthread_cond_broadcast(&z->cond);
            return NULL;
        }
        printNumber(z, z->curr);
        z->curr++;
        z->turn = 0;
        pthread_mutex_unlock(&z->mutex);
        pthread_cond_broadcast(&z->cond);
    }
    return NULL;
}

char* run_zero_even_odd(int n) {
    ZeroEvenOdd z;
    z.n = n;
    z.curr = 1;
    z.turn = 0;
    z.cap = (size_t)(n * 5 + 1);
    z.buf = (char*)calloc(z.cap, 1);
    z.pos = 0;
    pthread_mutex_init(&z.mutex, NULL);
    pthread_cond_init(&z.cond, NULL);

    pthread_t t0, t1, t2;
    pthread_create(&t0, NULL, zero_thread, &z);
    pthread_create(&t1, NULL, even_thread, &z);
    pthread_create(&t2, NULL, odd_thread, &z);

    pthread_join(t0, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    pthread_mutex_destroy(&z.mutex);
    pthread_cond_destroy(&z.cond);

    // Return the buffer (caller must free)
    return z.buf;
}

int main(void) {
    int tests[5] = {1, 2, 5, 10, 3};
    for (int i = 0; i < 5; ++i) {
        char* out = run_zero_even_odd(tests[i]);
        (void)out; // output already printed; buffer available if needed
        printf("\n");
        free(out);
    }
    return 0;
}