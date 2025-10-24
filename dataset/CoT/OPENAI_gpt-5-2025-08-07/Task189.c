#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

typedef void (*print_fn)(int, void*);

typedef struct {
    int n;
    sem_t zero_sem;
    sem_t even_sem;
    sem_t odd_sem;
} ZeroEvenOdd;

typedef struct {
    ZeroEvenOdd* zeo;
    print_fn printer;
    void* ctx;
} ThreadArgs;

typedef struct {
    char* data;
    size_t len;
    size_t cap;
    pthread_mutex_t lock;
} StrBuf;

static void sb_init(StrBuf* sb) {
    sb->cap = 64;
    sb->len = 0;
    sb->data = (char*)malloc(sb->cap);
    if (sb->data) sb->data[0] = '\0';
    pthread_mutex_init(&sb->lock, NULL);
}

static void sb_free(StrBuf* sb) {
    if (sb->data) free(sb->data);
    sb->data = NULL;
    sb->len = sb->cap = 0;
    pthread_mutex_destroy(&sb->lock);
}

static int sb_ensure(StrBuf* sb, size_t need) {
    if (need <= sb->cap) return 1;
    size_t newcap = sb->cap;
    while (newcap < need) {
        if (newcap > (SIZE_MAX / 2)) {
            newcap = need;
            break;
        }
        newcap *= 2;
    }
    char* nd = (char*)realloc(sb->data, newcap);
    if (!nd) return 0;
    sb->data = nd;
    sb->cap = newcap;
    return 1;
}

static void sb_append_int(StrBuf* sb, int x) {
    char tmp[32];
    int w = snprintf(tmp, sizeof(tmp), "%d", x);
    if (w <= 0) return;
    pthread_mutex_lock(&sb->lock);
    size_t need = sb->len + (size_t)w + 1;
    if (!sb_ensure(sb, need)) {
        pthread_mutex_unlock(&sb->lock);
        return;
    }
    memcpy(sb->data + sb->len, tmp, (size_t)w);
    sb->len += (size_t)w;
    sb->data[sb->len] = '\0';
    pthread_mutex_unlock(&sb->lock);
}

static int ZeroEvenOdd_init(ZeroEvenOdd* z, int n) {
    if (n < 1 || n > 1000) return 0;
    z->n = n;
    if (sem_init(&z->zero_sem, 0, 1) != 0) return 0;
    if (sem_init(&z->even_sem, 0, 0) != 0) return 0;
    if (sem_init(&z->odd_sem, 0, 0) != 0) return 0;
    return 1;
}

static void ZeroEvenOdd_destroy(ZeroEvenOdd* z) {
    sem_destroy(&z->zero_sem);
    sem_destroy(&z->even_sem);
    sem_destroy(&z->odd_sem);
}

static void* zero_thread(void* arg) {
    ThreadArgs* a = (ThreadArgs*)arg;
    ZeroEvenOdd* z = a->zeo;
    for (int i = 1; i <= z->n; ++i) {
        sem_wait(&z->zero_sem);
        a->printer(0, a->ctx);
        if (i % 2 == 1) {
            sem_post(&z->odd_sem);
        } else {
            sem_post(&z->even_sem);
        }
    }
    return NULL;
}

static void* even_thread(void* arg) {
    ThreadArgs* a = (ThreadArgs*)arg;
    ZeroEvenOdd* z = a->zeo;
    for (int i = 2; i <= z->n; i += 2) {
        sem_wait(&z->even_sem);
        a->printer(i, a->ctx);
        sem_post(&z->zero_sem);
    }
    return NULL;
}

static void* odd_thread(void* arg) {
    ThreadArgs* a = (ThreadArgs*)arg;
    ZeroEvenOdd* z = a->zeo;
    for (int i = 1; i <= z->n; i += 2) {
        sem_wait(&z->odd_sem);
        a->printer(i, a->ctx);
        sem_post(&z->zero_sem);
    }
    return NULL;
}

static void sb_printer(int x, void* ctx) {
    StrBuf* sb = (StrBuf*)ctx;
    sb_append_int(sb, x);
}

char* run_once(int n) {
    ZeroEvenOdd z;
    if (!ZeroEvenOdd_init(&z, n)) {
        char* err = (char*)malloc(1);
        if (err) err[0] = '\0';
        return err;
    }

    StrBuf sb;
    sb_init(&sb);

    ThreadArgs args;
    args.zeo = &z;
    args.printer = sb_printer;
    args.ctx = &sb;

    pthread_t t0, te, to;
    if (pthread_create(&t0, NULL, zero_thread, &args) != 0 ||
        pthread_create(&te, NULL, even_thread, &args) != 0 ||
        pthread_create(&to, NULL, odd_thread, &args) != 0) {
        // Cleanup on failure
        ZeroEvenOdd_destroy(&z);
        char* res = (char*)malloc(1);
        if (res) res[0] = '\0';
        sb_free(&sb);
        return res;
    }

    pthread_join(t0, NULL);
    pthread_join(te, NULL);
    pthread_join(to, NULL);

    char* result = (char*)malloc(sb.len + 1);
    if (result) {
        memcpy(result, sb.data, sb.len + 1);
    }

    sb_free(&sb);
    ZeroEvenOdd_destroy(&z);
    return result;
}

int main(void) {
    int tests[5] = {1, 2, 5, 7, 10};
    for (int i = 0; i < 5; ++i) {
        int n = tests[i];
        char* out = run_once(n);
        if (out) {
            printf("n=%d: %s\n", n, out);
            free(out);
        } else {
            printf("n=%d: \n", n);
        }
    }
    return 0;
}