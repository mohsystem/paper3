#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct {
    int n;
    int cur;
    int zero_turn; // 1 if zero should print, 0 otherwise
    pthread_mutex_t mtx;
    pthread_cond_t cv;
} ZeroEvenOdd;

typedef struct {
    char *buf;
    size_t len;
    size_t cap;
    pthread_mutex_t mtx;
} OutputBuffer;

typedef struct {
    ZeroEvenOdd *zeo;
    void (*printNumber)(int, void *);
    void *ctx;
} ThreadArgs;

static void OutputBuffer_init(OutputBuffer *ob, size_t cap) {
    ob->buf = (char *)malloc(cap);
    if (!ob->buf) {
        fprintf(stderr, "Allocation failed\n");
        exit(1);
    }
    ob->buf[0] = '\0';
    ob->len = 0;
    ob->cap = cap;
    pthread_mutex_init(&ob->mtx, NULL);
}

static void OutputBuffer_destroy(OutputBuffer *ob) {
    pthread_mutex_destroy(&ob->mtx);
    free(ob->buf);
}

static void OutputBuffer_append(OutputBuffer *ob, const char *s) {
    pthread_mutex_lock(&ob->mtx);
    size_t slen = strlen(s);
    if (slen >= ob->cap - ob->len) {
        slen = (ob->cap - ob->len - 1);
    }
    if (slen > 0) {
        memcpy(ob->buf + ob->len, s, slen);
        ob->len += slen;
        ob->buf[ob->len] = '\0';
    }
    pthread_mutex_unlock(&ob->mtx);
}

static void safe_printNumber(int x, void *ctx) {
    OutputBuffer *ob = (OutputBuffer *)ctx;
    char tmp[16];
    int written = snprintf(tmp, sizeof(tmp), "%d", x);
    if (written > 0) {
        OutputBuffer_append(ob, tmp);
    }
    printf("%d", x);
    fflush(stdout);
}

static void ZeroEvenOdd_init(ZeroEvenOdd *zeo, int n) {
    if (n < 1 || n > 1000) {
        fprintf(stderr, "n must be in [1, 1000]\n");
        exit(1);
    }
    zeo->n = n;
    zeo->cur = 1;
    zeo->zero_turn = 1;
    pthread_mutex_init(&zeo->mtx, NULL);
    pthread_cond_init(&zeo->cv, NULL);
}

static void ZeroEvenOdd_destroy(ZeroEvenOdd *zeo) {
    pthread_mutex_destroy(&zeo->mtx);
    pthread_cond_destroy(&zeo->cv);
}

static void ZeroEvenOdd_zero(ZeroEvenOdd *zeo, void (*printNumber)(int, void *), void *ctx) {
    pthread_mutex_lock(&zeo->mtx);
    for (;;) {
        while (!(zeo->zero_turn) && zeo->cur <= zeo->n) {
            pthread_cond_wait(&zeo->cv, &zeo->mtx);
        }
        if (zeo->cur > zeo->n) {
            pthread_cond_broadcast(&zeo->cv);
            pthread_mutex_unlock(&zeo->mtx);
            return;
        }
        printNumber(0, ctx);
        zeo->zero_turn = 0;
        pthread_cond_broadcast(&zeo->cv);
    }
}

static void ZeroEvenOdd_even(ZeroEvenOdd *zeo, void (*printNumber)(int, void *), void *ctx) {
    pthread_mutex_lock(&zeo->mtx);
    for (;;) {
        while (!(zeo->cur > zeo->n || (!zeo->zero_turn && (zeo->cur % 2 == 0)))) {
            pthread_cond_wait(&zeo->cv, &zeo->mtx);
        }
        if (zeo->cur > zeo->n) {
            pthread_cond_broadcast(&zeo->cv);
            pthread_mutex_unlock(&zeo->mtx);
            return;
        }
        printNumber(zeo->cur, ctx);
        zeo->cur++;
        zeo->zero_turn = 1;
        pthread_cond_broadcast(&zeo->cv);
    }
}

static void ZeroEvenOdd_odd(ZeroEvenOdd *zeo, void (*printNumber)(int, void *), void *ctx) {
    pthread_mutex_lock(&zeo->mtx);
    for (;;) {
        while (!(zeo->cur > zeo->n || (!zeo->zero_turn && (zeo->cur % 2 == 1)))) {
            pthread_cond_wait(&zeo->cv, &zeo->mtx);
        }
        if (zeo->cur > zeo->n) {
            pthread_cond_broadcast(&zeo->cv);
            pthread_mutex_unlock(&zeo->mtx);
            return;
        }
        printNumber(zeo->cur, ctx);
        zeo->cur++;
        zeo->zero_turn = 1;
        pthread_cond_broadcast(&zeo->cv);
    }
}

typedef struct {
    ZeroEvenOdd *zeo;
    void (*printNumber)(int, void *);
    void *ctx;
    int role; // 0=zero,1=even,2=odd
} WorkerArgs;

static void *worker(void *arg) {
    WorkerArgs *wa = (WorkerArgs *)arg;
    if (wa->role == 0) {
        ZeroEvenOdd_zero(wa->zeo, wa->printNumber, wa->ctx);
    } else if (wa->role == 1) {
        ZeroEvenOdd_even(wa->zeo, wa->printNumber, wa->ctx);
    } else {
        ZeroEvenOdd_odd(wa->zeo, wa->printNumber, wa->ctx);
    }
    return NULL;
}

static void run_test(int n) {
    ZeroEvenOdd zeo;
    ZeroEvenOdd_init(&zeo, n);

    OutputBuffer ob;
    OutputBuffer_init(&ob, 8192);

    WorkerArgs a0 = { &zeo, safe_printNumber, &ob, 0 };
    WorkerArgs a1 = { &zeo, safe_printNumber, &ob, 1 };
    WorkerArgs a2 = { &zeo, safe_printNumber, &ob, 2 };

    pthread_t t0, t1, t2;
    pthread_create(&t0, NULL, worker, &a0);
    pthread_create(&t1, NULL, worker, &a1);
    pthread_create(&t2, NULL, worker, &a2);

    pthread_join(t0, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf(" -> %s\n", ob.buf);

    OutputBuffer_destroy(&ob);
    ZeroEvenOdd_destroy(&zeo);
}

int main(void) {
    run_test(1);
    run_test(2);
    run_test(3);
    run_test(5);
    run_test(10);
    return 0;
}