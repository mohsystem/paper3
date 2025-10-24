#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct {
    char* data;
    size_t size;
    size_t capacity;
    pthread_mutex_t lock;
} Buffer;

static int buffer_init(Buffer* b) {
    b->capacity = 64;
    b->size = 0;
    b->data = (char*)malloc(b->capacity);
    if (!b->data) return -1;
    b->data[0] = '\0';
    if (pthread_mutex_init(&b->lock, NULL) != 0) {
        free(b->data);
        b->data = NULL;
        return -1;
    }
    return 0;
}

static void buffer_free(Buffer* b) {
    if (b->data) free(b->data);
    pthread_mutex_destroy(&b->lock);
}

static int buffer_reserve(Buffer* b, size_t extra) {
    if (b->size + extra + 1 <= b->capacity) return 0;
    size_t newcap = b->capacity;
    while (b->size + extra + 1 > newcap) {
        newcap *= 2;
    }
    char* nd = (char*)realloc(b->data, newcap);
    if (!nd) return -1;
    b->data = nd;
    b->capacity = newcap;
    return 0;
}

static int buffer_append_str(Buffer* b, const char* s) {
    size_t len = strlen(s);
    if (buffer_reserve(b, len) != 0) return -1;
    memcpy(b->data + b->size, s, len);
    b->size += len;
    b->data[b->size] = '\0';
    return 0;
}

static int int_to_str(int x, char* tmp, size_t tmpsz) {
    // returns length written
    int len = snprintf(tmp, tmpsz, "%d", x);
    if (len < 0 || (size_t)len >= tmpsz) return -1;
    return len;
}

typedef struct {
    int n;
    pthread_mutex_t mtx;
    pthread_cond_t cv;
    int state; // 0 zero's turn, 1 odd's turn, 2 even's turn
} ZeroEvenOdd;

static int zeo_init(ZeroEvenOdd* z, int n) {
    if (n < 1 || n > 1000) return -1;
    z->n = n;
    z->state = 0;
    if (pthread_mutex_init(&z->mtx, NULL) != 0) return -1;
    if (pthread_cond_init(&z->cv, NULL) != 0) {
        pthread_mutex_destroy(&z->mtx);
        return -1;
    }
    return 0;
}

static void zeo_destroy(ZeroEvenOdd* z) {
    pthread_cond_destroy(&z->cv);
    pthread_mutex_destroy(&z->mtx);
}

typedef void (*print_fn)(int, void*);

typedef struct {
    ZeroEvenOdd* zeo;
    print_fn printer;
    void* ctx;
} ThreadArgs;

static void zero_func(ZeroEvenOdd* zeo, print_fn printer, void* ctx) {
    for (int i = 1; i <= zeo->n; ++i) {
        pthread_mutex_lock(&zeo->mtx);
        while (zeo->state != 0) {
            pthread_cond_wait(&zeo->cv, &zeo->mtx);
        }
        printer(0, ctx);
        zeo->state = (i % 2 == 1) ? 1 : 2;
        pthread_mutex_unlock(&zeo->mtx);
        pthread_cond_broadcast(&zeo->cv);
    }
}

static void even_func(ZeroEvenOdd* zeo, print_fn printer, void* ctx) {
    for (int i = 2; i <= zeo->n; i += 2) {
        pthread_mutex_lock(&zeo->mtx);
        while (zeo->state != 2) {
            pthread_cond_wait(&zeo->cv, &zeo->mtx);
        }
        printer(i, ctx);
        zeo->state = 0;
        pthread_mutex_unlock(&zeo->mtx);
        pthread_cond_broadcast(&zeo->cv);
    }
}

static void odd_func(ZeroEvenOdd* zeo, print_fn printer, void* ctx) {
    for (int i = 1; i <= zeo->n; i += 2) {
        pthread_mutex_lock(&zeo->mtx);
        while (zeo->state != 1) {
            pthread_cond_wait(&zeo->cv, &zeo->mtx);
        }
        printer(i, ctx);
        zeo->state = 0;
        pthread_mutex_unlock(&zeo->mtx);
        pthread_cond_broadcast(&zeo->cv);
    }
}

static void printNumberC(int x, void* ctx) {
    Buffer* buf = (Buffer*)ctx;
    char tmp[16];
    int len = int_to_str(x, tmp, sizeof(tmp));
    if (len < 0) return;
    pthread_mutex_lock(&buf->lock);
    printf("%s", tmp);
    fflush(stdout);
    buffer_append_str(buf, tmp);
    pthread_mutex_unlock(&buf->lock);
}

static void* thread_start_zero(void* arg) {
    ThreadArgs* a = (ThreadArgs*)arg;
    zero_func(a->zeo, a->printer, a->ctx);
    return NULL;
}

static void* thread_start_even(void* arg) {
    ThreadArgs* a = (ThreadArgs*)arg;
    even_func(a->zeo, a->printer, a->ctx);
    return NULL;
}

static void* thread_start_odd(void* arg) {
    ThreadArgs* a = (ThreadArgs*)arg;
    odd_func(a->zeo, a->printer, a->ctx);
    return NULL;
}

char* run_zero_even_odd_c(int n) {
    ZeroEvenOdd zeo;
    if (zeo_init(&zeo, n) != 0) {
        return NULL;
    }
    Buffer buf;
    if (buffer_init(&buf) != 0) {
        zeo_destroy(&zeo);
        return NULL;
    }

    ThreadArgs args;
    args.zeo = &zeo;
    args.printer = printNumberC;
    args.ctx = &buf;

    pthread_t tZero, tEven, tOdd;
    if (pthread_create(&tZero, NULL, thread_start_zero, &args) != 0 ||
        pthread_create(&tEven, NULL, thread_start_even, &args) != 0 ||
        pthread_create(&tOdd, NULL, thread_start_odd, &args) != 0) {
        // Best effort cleanup
        // Joining only created threads is complex; keep it simple for this context
        zeo_destroy(&zeo);
        buffer_free(&buf);
        return NULL;
    }

    pthread_join(tZero, NULL);
    pthread_join(tEven, NULL);
    pthread_join(tOdd, NULL);

    printf("\n");

    char* result = (char*)malloc(buf.size + 1);
    if (result) {
        memcpy(result, buf.data, buf.size + 1);
    }

    zeo_destroy(&zeo);
    buffer_free(&buf);
    return result;
}

int main(void) {
    int tests[5] = {1, 2, 5, 10, 3};
    for (int i = 0; i < 5; ++i) {
        char* out = run_zero_even_odd_c(tests[i]);
        if (out) {
            printf("Returned: %s\n", out);
            free(out);
        } else {
            printf("Error running test with n=%d\n", tests[i]);
        }
    }
    return 0;
}