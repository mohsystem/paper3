#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

typedef void (*print_fn)(int x, void *user);

typedef struct {
    int n;
    sem_t sem_zero;
    sem_t sem_odd;
    sem_t sem_even;
} ZeroEvenOdd;

typedef struct {
    ZeroEvenOdd *zeo;
    print_fn printer;
    void *user;
} ThreadCtx;

typedef struct {
    char *buf;
    size_t cap;
    size_t pos;
    pthread_mutex_t lock;
} OutputBuf;

static void safe_append(OutputBuf *ob, const char *s) {
    pthread_mutex_lock(&ob->lock);
    size_t len = strlen(s);
    if (ob->pos + len < ob->cap) {
        memcpy(ob->buf + ob->pos, s, len);
        ob->pos += len;
        ob->buf[ob->pos] = '\0';
    }
    pthread_mutex_unlock(&ob->lock);
}

static void printNumber_cb(int x, void *user) {
    char tmp[16];
    snprintf(tmp, sizeof(tmp), "%d", x);
    safe_append((OutputBuf *)user, tmp);
}

static void ZeroEvenOdd_init(ZeroEvenOdd *z, int n) {
    z->n = n;
    sem_init(&z->sem_zero, 0, 1);
    sem_init(&z->sem_odd, 0, 0);
    sem_init(&z->sem_even, 0, 0);
}

static void ZeroEvenOdd_destroy(ZeroEvenOdd *z) {
    sem_destroy(&z->sem_zero);
    sem_destroy(&z->sem_odd);
    sem_destroy(&z->sem_even);
}

static void zero_func(ZeroEvenOdd *z, print_fn printer, void *user) {
    for (int i = 1; i <= z->n; i++) {
        sem_wait(&z->sem_zero);
        printer(0, user);
        if ((i & 1) == 1) sem_post(&z->sem_odd);
        else sem_post(&z->sem_even);
    }
}

static void odd_func(ZeroEvenOdd *z, print_fn printer, void *user) {
    for (int i = 1; i <= z->n; i += 2) {
        sem_wait(&z->sem_odd);
        printer(i, user);
        sem_post(&z->sem_zero);
    }
}

static void even_func(ZeroEvenOdd *z, print_fn printer, void *user) {
    for (int i = 2; i <= z->n; i += 2) {
        sem_wait(&z->sem_even);
        printer(i, user);
        sem_post(&z->sem_zero);
    }
}

static void *zero_thread(void *arg) {
    ThreadCtx *ctx = (ThreadCtx *)arg;
    zero_func(ctx->zeo, ctx->printer, ctx->user);
    return NULL;
}
static void *odd_thread(void *arg) {
    ThreadCtx *ctx = (ThreadCtx *)arg;
    odd_func(ctx->zeo, ctx->printer, ctx->user);
    return NULL;
}
static void *even_thread(void *arg) {
    ThreadCtx *ctx = (ThreadCtx *)arg;
    even_func(ctx->zeo, ctx->printer, ctx->user);
    return NULL;
}

static size_t total_digits_upto(int n) {
    size_t total = 0;
    int start = 1;
    int digits = 1;
    while (start <= n) {
        int end = (int)fmin(n, (int)(start * 10 - 1));
        total += (size_t)(end - start + 1) * (size_t)digits;
        start *= 10;
        digits++;
    }
    return total;
}

char *run_zero_even_odd(int n) {
    if (n < 1 || n > 1000) {
        char *empty = (char *)calloc(1, 1);
        return empty ? empty : NULL;
    }
    ZeroEvenOdd z;
    ZeroEvenOdd_init(&z, n);

    size_t cap = (size_t)n /* zeros */ + total_digits_upto(n) + 1;
    if (cap < 64) cap = 64;
    OutputBuf ob;
    ob.buf = (char *)malloc(cap);
    if (!ob.buf) {
        ZeroEvenOdd_destroy(&z);
        return NULL;
    }
    ob.cap = cap;
    ob.pos = 0;
    ob.buf[0] = '\0';
    pthread_mutex_init(&ob.lock, NULL);

    ThreadCtx ctx = { .zeo = &z, .printer = printNumber_cb, .user = &ob };

    pthread_t t0, t1, t2;
    pthread_create(&t0, NULL, zero_thread, &ctx);
    pthread_create(&t1, NULL, odd_thread, &ctx);
    pthread_create(&t2, NULL, even_thread, &ctx);

    pthread_join(t0, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    ZeroEvenOdd_destroy(&z);
    pthread_mutex_destroy(&ob.lock);

    // Trim buffer to exact size
    char *res = (char *)malloc(ob.pos + 1);
    if (res) {
        memcpy(res, ob.buf, ob.pos + 1);
    }
    free(ob.buf);
    return res;
}

int main(void) {
    int tests[5] = {1, 2, 5, 7, 10};
    for (int i = 0; i < 5; i++) {
        char *s = run_zero_even_odd(tests[i]);
        if (s) {
            printf("%s\n", s);
            free(s);
        } else {
            printf("\n");
        }
    }
    return 0;
}