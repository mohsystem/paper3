#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/* Simple counting semaphore */
typedef struct {
    int permits;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Semaphore;

void semaphore_init(Semaphore* s, int initial) {
    s->permits = initial;
    pthread_mutex_init(&s->mutex, NULL);
    pthread_cond_init(&s->cond, NULL);
}

void semaphore_acquire(Semaphore* s) {
    pthread_mutex_lock(&s->mutex);
    while (s->permits == 0) {
        pthread_cond_wait(&s->cond, &s->mutex);
    }
    s->permits--;
    pthread_mutex_unlock(&s->mutex);
}

void semaphore_release(Semaphore* s) {
    pthread_mutex_lock(&s->mutex);
    s->permits++;
    pthread_cond_signal(&s->cond);
    pthread_mutex_unlock(&s->mutex);
}

void semaphore_destroy(Semaphore* s) {
    pthread_mutex_destroy(&s->mutex);
    pthread_cond_destroy(&s->cond);
}

/* Reusable barrier for N threads */
typedef struct {
    int threshold;
    int count;
    int generation;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Barrier;

void barrier_init(Barrier* b, int threshold) {
    b->threshold = threshold;
    b->count = 0;
    b->generation = 0;
    pthread_mutex_init(&b->mutex, NULL);
    pthread_cond_init(&b->cond, NULL);
}

void barrier_wait(Barrier* b) {
    pthread_mutex_lock(&b->mutex);
    int gen = b->generation;
    b->count++;
    if (b->count == b->threshold) {
        b->generation++;
        b->count = 0;
        pthread_cond_broadcast(&b->cond);
        pthread_mutex_unlock(&b->mutex);
        return;
    }
    while (gen == b->generation) {
        pthread_cond_wait(&b->cond, &b->mutex);
    }
    pthread_mutex_unlock(&b->mutex);
}

void barrier_destroy(Barrier* b) {
    pthread_mutex_destroy(&b->mutex);
    pthread_cond_destroy(&b->cond);
}

/* H2O synchronization */
typedef struct {
    Semaphore hSem;
    Semaphore oSem;
    Barrier barrier;
} H2O;

void h2o_init(H2O* h2o) {
    semaphore_init(&h2o->hSem, 2);
    semaphore_init(&h2o->oSem, 1);
    barrier_init(&h2o->barrier, 3);
}
void h2o_destroy(H2O* h2o) {
    semaphore_destroy(&h2o->hSem);
    semaphore_destroy(&h2o->oSem);
    barrier_destroy(&h2o->barrier);
}

typedef struct {
    char* buf;
    int len;
    int idx;
    pthread_mutex_t mutex;
} AppendBuffer;

void append_buffer_init(AppendBuffer* ab, int len) {
    ab->buf = (char*)malloc((size_t)len + 1);
    ab->len = len;
    ab->idx = 0;
    pthread_mutex_init(&ab->mutex, NULL);
}
void append_buffer_destroy(AppendBuffer* ab) {
    pthread_mutex_destroy(&ab->mutex);
    // caller frees buf
}

void releaseHydrogen(void* arg) {
    AppendBuffer* ab = (AppendBuffer*)arg;
    pthread_mutex_lock(&ab->mutex);
    if (ab->idx < ab->len) {
        ab->buf[ab->idx++] = 'H';
    }
    pthread_mutex_unlock(&ab->mutex);
}

void releaseOxygen(void* arg) {
    AppendBuffer* ab = (AppendBuffer*)arg;
    pthread_mutex_lock(&ab->mutex);
    if (ab->idx < ab->len) {
        ab->buf[ab->idx++] = 'O';
    }
    pthread_mutex_unlock(&ab->mutex);
}

void h2o_hydrogen(H2O* h2o, void (*releaseHydrogenFn)(void*), void* arg) {
    semaphore_acquire(&h2o->hSem);
    releaseHydrogenFn(arg);
    barrier_wait(&h2o->barrier);
    semaphore_release(&h2o->hSem);
}

void h2o_oxygen(H2O* h2o, void (*releaseOxygenFn)(void*), void* arg) {
    semaphore_acquire(&h2o->oSem);
    releaseOxygenFn(arg);
    barrier_wait(&h2o->barrier);
    semaphore_release(&h2o->oSem);
}

typedef struct {
    H2O* h2o;
    AppendBuffer* out;
} ThreadArg;

void* hydrogen_thread(void* p) {
    ThreadArg* ta = (ThreadArg*)p;
    h2o_hydrogen(ta->h2o, releaseHydrogen, ta->out);
    return NULL;
}
void* oxygen_thread(void* p) {
    ThreadArg* ta = (ThreadArg*)p;
    h2o_oxygen(ta->h2o, releaseOxygen, ta->out);
    return NULL;
}

char* run_water(const char* water) {
    if (water == NULL) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    int n = (int)strlen(water);
    H2O h2o;
    h2o_init(&h2o);

    AppendBuffer out;
    append_buffer_init(&out, n);

    pthread_t* tids = (pthread_t*)malloc(sizeof(pthread_t) * (size_t)n);
    ThreadArg ta;
    ta.h2o = &h2o;
    ta.out = &out;

    int tcount = 0;
    for (int i = 0; i < n; ++i) {
        if (water[i] == 'H') {
            pthread_create(&tids[tcount++], NULL, hydrogen_thread, &ta);
        } else if (water[i] == 'O') {
            pthread_create(&tids[tcount++], NULL, oxygen_thread, &ta);
        } else {
            // ignore
        }
    }
    for (int i = 0; i < tcount; ++i) {
        pthread_join(tids[i], NULL);
    }

    if (out.idx < out.len) {
        // ensure full length filled if any skipped; pad with '\0' after
    }
    out.buf[out.len] = '\0';

    free(tids);
    append_buffer_destroy(&out);
    h2o_destroy(&h2o);
    return out.buf; // caller must free
}

int validate_water(const char* out) {
    if (!out) return 0;
    int len = (int)strlen(out);
    if (len % 3 != 0) return 0;
    for (int i = 0; i < len; i += 3) {
        int h = 0, o = 0;
        for (int j = i; j < i + 3; ++j) {
            if (out[j] == 'H') h++;
            else if (out[j] == 'O') o++;
            else return 0;
        }
        if (!(h == 2 && o == 1)) return 0;
    }
    return 1;
}

int main(void) {
    const char* tests[5] = {
        "HOH",
        "OOHHHH",
        "HHOHHO",
        "OHHHOH",
        "HHOOHH"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = run_water(tests[i]);
        printf("Input:  %s\n", tests[i]);
        printf("Output: %s | valid=%s\n", res ? res : "", validate_water(res) ? "true" : "false");
        printf("---\n");
        free(res);
    }
    return 0;
}