#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

/* Simple counting semaphore */
typedef struct {
    int count;
    pthread_mutex_t mtx;
    pthread_cond_t cv;
} MySemaphore;

static void sem_init_my(MySemaphore* s, int initial) {
    s->count = initial;
    pthread_mutex_init(&s->mtx, NULL);
    pthread_cond_init(&s->cv, NULL);
}
static void sem_acquire(MySemaphore* s) {
    pthread_mutex_lock(&s->mtx);
    while (s->count <= 0) {
        pthread_cond_wait(&s->cv, &s->mtx);
    }
    s->count--;
    pthread_mutex_unlock(&s->mtx);
}
static void sem_release_n(MySemaphore* s, int n) {
    pthread_mutex_lock(&s->mtx);
    s->count += n;
    pthread_mutex_unlock(&s->mtx);
    pthread_cond_broadcast(&s->cv);
}
static void sem_destroy_my(MySemaphore* s) {
    pthread_mutex_destroy(&s->mtx);
    pthread_cond_destroy(&s->cv);
}

/* Reusable barrier for 3 threads */
typedef struct {
    int total;
    int count;
    int generation;
    pthread_mutex_t mtx;
    pthread_cond_t cv;
} MyBarrier;

static void barrier_init(MyBarrier* b, int total) {
    b->total = total;
    b->count = 0;
    b->generation = 0;
    pthread_mutex_init(&b->mtx, NULL);
    pthread_cond_init(&b->cv, NULL);
}
static int barrier_wait_last(MyBarrier* b) {
    pthread_mutex_lock(&b->mtx);
    int gen = b->generation;
    b->count++;
    int last = 0;
    if (b->count == b->total) {
        b->generation++;
        b->count = 0;
        last = 1;
        pthread_mutex_unlock(&b->mtx);
        pthread_cond_broadcast(&b->cv);
    } else {
        while (gen == b->generation) {
            pthread_cond_wait(&b->cv, &b->mtx);
        }
        pthread_mutex_unlock(&b->mtx);
    }
    return last;
}
static void barrier_destroy(MyBarrier* b) {
    pthread_mutex_destroy(&b->mtx);
    pthread_cond_destroy(&b->cv);
}

/* H2O synchronization */
typedef struct {
    MySemaphore hSem;
    MySemaphore oSem;
    MyBarrier barrier;
} H2O;

static void h2o_init(H2O* h) {
    sem_init_my(&h->hSem, 2);
    sem_init_my(&h->oSem, 1);
    barrier_init(&h->barrier, 3);
}
static void h2o_destroy(H2O* h) {
    sem_destroy_my(&h->hSem);
    sem_destroy_my(&h->oSem);
    barrier_destroy(&h->barrier);
}

/* Output buffer */
typedef struct {
    char* buf;
    size_t cap;
    size_t idx;
    pthread_mutex_t mtx;
} OutBuf;

static void outbuf_init(OutBuf* ob, size_t cap) {
    ob->buf = (char*)malloc(cap + 1);
    ob->cap = cap;
    ob->idx = 0;
    pthread_mutex_init(&ob->mtx, NULL);
}
static void outbuf_append(OutBuf* ob, char c) {
    pthread_mutex_lock(&ob->mtx);
    if (ob->idx < ob->cap) {
        ob->buf[ob->idx++] = c;
    }
    pthread_mutex_unlock(&ob->mtx);
}
static void outbuf_finalize(OutBuf* ob) {
    if (ob->buf) ob->buf[ob->idx] = '\0';
}
static void outbuf_destroy(OutBuf* ob) {
    pthread_mutex_destroy(&ob->mtx);
    /* caller frees ob->buf if needed */
}

/* Thread arg */
typedef struct {
    H2O* h2o;
    OutBuf* out;
    char type; /* 'H' or 'O' */
} ThreadArg;

static void* hydrogen_thread(void* arg) {
    ThreadArg* a = (ThreadArg*)arg;
    H2O* h = a->h2o;
    sem_acquire(&h->hSem);
    outbuf_append(a->out, 'H');
    int last = barrier_wait_last(&h->barrier);
    if (last) {
        sem_release_n(&h->hSem, 2);
        sem_release_n(&h->oSem, 1);
    }
    return NULL;
}

static void* oxygen_thread(void* arg) {
    ThreadArg* a = (ThreadArg*)arg;
    H2O* h = a->h2o;
    sem_acquire(&h->oSem);
    outbuf_append(a->out, 'O');
    int last = barrier_wait_last(&h->barrier);
    if (last) {
        sem_release_n(&h->hSem, 2);
        sem_release_n(&h->oSem, 1);
    }
    return NULL;
}

static int validate_molecules(const char* s) {
    if (!s) return 0;
    size_t len = strlen(s);
    if (len % 3 != 0) return 0;
    for (size_t i = 0; i < len; i += 3) {
        int h = 0, o = 0;
        for (size_t j = 0; j < 3; ++j) {
            char c = s[i + j];
            if (c == 'H') h++;
            else if (c == 'O') o++;
            else return 0;
        }
        if (!(h == 2 && o == 1)) return 0;
    }
    return 1;
}

/* Returns a newly allocated string which caller must free */
char* form_water(const char* water) {
    if (!water) return NULL;
    size_t n = strlen(water);
    if (n % 3 != 0) return NULL;

    size_t h = 0, o = 0;
    for (size_t i = 0; i < n; ++i) {
        if (water[i] == 'H') h++;
        else if (water[i] == 'O') o++;
        else return NULL;
    }
    if (h != 2 * o) return NULL;

    H2O h2o;
    h2o_init(&h2o);

    OutBuf out;
    outbuf_init(&out, n);

    pthread_t* tids = (pthread_t*)malloc(n * sizeof(pthread_t));
    ThreadArg* args = (ThreadArg*)malloc(n * sizeof(ThreadArg));
    if (!tids || !args) {
        free(tids);
        free(args);
        h2o_destroy(&h2o);
        outbuf_destroy(&out);
        free(out.buf);
        return NULL;
    }

    size_t idx = 0;
    for (size_t i = 0; i < n; ++i) {
        args[idx].h2o = &h2o;
        args[idx].out = &out;
        args[idx].type = water[i];
        if (water[i] == 'H') {
            pthread_create(&tids[idx], NULL, hydrogen_thread, &args[idx]);
        } else {
            pthread_create(&tids[idx], NULL, oxygen_thread, &args[idx]);
        }
        idx++;
    }

    for (size_t i = 0; i < n; ++i) {
        pthread_join(tids[i], NULL);
    }

    outbuf_finalize(&out);

    free(tids);
    free(args);
    h2o_destroy(&h2o);

    /* caller will free out.buf */
    return out.buf;
}

int main(void) {
    const char* tests[5] = {
        "HOH",
        "OOHHHH",
        "HHOOHH",
        "OOOHHHHHH",
        "HHOHHOHHO"
    };
    for (int i = 0; i < 5; ++i) {
        char* out = form_water(tests[i]);
        printf("Input:  %s\n", tests[i]);
        if (out) {
            printf("Output: %s\n", out);
            printf("Valid:  %d\n", validate_molecules(out));
            free(out);
        } else {
            printf("Output: (null)\n");
            printf("Valid:  0\n");
        }
        printf("---\n");
    }
    return 0;
}