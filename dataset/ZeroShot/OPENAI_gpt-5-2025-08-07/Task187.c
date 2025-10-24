#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cvEnter;
    pthread_cond_t cvBarrier;
    int hSlots;
    int oSlots;
    int arrived;
    int gen;
} H2O;

typedef struct {
    char* buf;
    size_t len;
    size_t pos;
    pthread_mutex_t lock;
} Collector;

typedef struct {
    H2O* h2o;
    Collector* col;
} ThreadArg;

static void h2o_init(H2O* h) {
    pthread_mutex_init(&h->mutex, NULL);
    pthread_cond_init(&h->cvEnter, NULL);
    pthread_cond_init(&h->cvBarrier, NULL);
    h->hSlots = 2;
    h->oSlots = 1;
    h->arrived = 0;
    h->gen = 0;
}

static void h2o_destroy(H2O* h) {
    pthread_mutex_destroy(&h->mutex);
    pthread_cond_destroy(&h->cvEnter);
    pthread_cond_destroy(&h->cvBarrier);
}

static void collector_init(Collector* c, size_t len) {
    c->buf = (char*)calloc(len + 1, 1);
    c->len = len;
    c->pos = 0;
    pthread_mutex_init(&c->lock, NULL);
}

static void collector_destroy(Collector* c) {
    pthread_mutex_destroy(&c->lock);
    free(c->buf);
}

static void collector_append(Collector* c, char ch) {
    pthread_mutex_lock(&c->lock);
    if (c->pos < c->len) {
        c->buf[c->pos++] = ch;
    }
    pthread_mutex_unlock(&c->lock);
}

static void h2o_hydrogen(H2O* h, void (*releaseHydrogen)(void*), void* arg) {
    int myGen;
    pthread_mutex_lock(&h->mutex);
    while (h->hSlots == 0) {
        pthread_cond_wait(&h->cvEnter, &h->mutex);
    }
    h->hSlots--;
    myGen = h->gen;
    pthread_mutex_unlock(&h->mutex);

    releaseHydrogen(arg);

    pthread_mutex_lock(&h->mutex);
    h->arrived++;
    if (h->arrived == 3) {
        h->arrived = 0;
        h->gen++;
        h->hSlots = 2;
        h->oSlots = 1;
        pthread_cond_broadcast(&h->cvEnter);
        pthread_cond_broadcast(&h->cvBarrier);
    } else {
        while (h->gen == myGen) {
            pthread_cond_wait(&h->cvBarrier, &h->mutex);
        }
    }
    pthread_mutex_unlock(&h->mutex);
}

static void h2o_oxygen(H2O* h, void (*releaseOxygen)(void*), void* arg) {
    int myGen;
    pthread_mutex_lock(&h->mutex);
    while (h->oSlots == 0) {
        pthread_cond_wait(&h->cvEnter, &h->mutex);
    }
    h->oSlots--;
    myGen = h->gen;
    pthread_mutex_unlock(&h->mutex);

    releaseOxygen(arg);

    pthread_mutex_lock(&h->mutex);
    h->arrived++;
    if (h->arrived == 3) {
        h->arrived = 0;
        h->gen++;
        h->hSlots = 2;
        h->oSlots = 1;
        pthread_cond_broadcast(&h->cvEnter);
        pthread_cond_broadcast(&h->cvBarrier);
    } else {
        while (h->gen == myGen) {
            pthread_cond_wait(&h->cvBarrier, &h->mutex);
        }
    }
    pthread_mutex_unlock(&h->mutex);
}

static void releaseH(void* arg) {
    Collector* c = (Collector*)arg;
    collector_append(c, 'H');
}

static void releaseO(void* arg) {
    Collector* c = (Collector*)arg;
    collector_append(c, 'O');
}

typedef struct {
    H2O* h2o;
    Collector* col;
} WorkerArgs;

typedef struct {
    H2O* h2o;
    Collector* col;
} WorkerArgs2;

typedef struct {
    H2O* h2o;
    Collector* col;
} WorkerArgs3;

typedef struct {
    H2O* h2o;
    Collector* col;
} WorkerArgs4;

typedef struct {
    H2O* h2o;
    Collector* col;
} WorkerArgs5;

typedef struct {
    H2O* h2o;
    Collector* col;
} WorkerArgs6;

typedef struct {
    H2O* h2o;
    Collector* col;
} WorkerArgs7;

typedef struct {
    H2O* h2o;
    Collector* col;
} WorkerArgs8;

typedef struct {
    H2O* h2o;
    Collector* col;
} WorkerArgs9;

typedef struct {
    H2O* h2o;
    Collector* col;
} WorkerArgs10;

typedef struct {
    H2O* h2o;
    Collector* col;
} WorkerArgs11;

typedef struct {
    H2O* h2o;
    Collector* col;
} WorkerArgs12;

typedef struct {
    H2O* h2o;
    Collector* col;
} WorkerArgs13;

typedef struct {
    H2O* h2o;
    Collector* col;
} WorkerArgs14;

typedef struct {
    H2O* h2o;
    Collector* col;
} WorkerArgs15;

typedef struct {
    H2O* h2o;
    Collector* col;
} WorkerArgs16;

typedef struct {
    H2O* h2o;
    Collector* col;
} WorkerArgs17;

typedef struct {
    H2O* h2o;
    Collector* col;
} WorkerArgs18;

typedef struct {
    H2O* h2o;
    Collector* col;
} WorkerArgs19;

typedef struct {
    H2O* h2o;
    Collector* col;
} WorkerArgs20;

// Thread wrappers
typedef struct {
    H2O* h2o;
    Collector* col;
    char type;
} TArg;

static void* hydrogen_thread(void* arg) {
    TArg* a = (TArg*)arg;
    h2o_hydrogen(a->h2o, releaseH, a->col);
    return NULL;
}

static void* oxygen_thread(void* arg) {
    TArg* a = (TArg*)arg;
    h2o_oxygen(a->h2o, releaseO, a->col);
    return NULL;
}

static char* build_water(const char* water) {
    if (!water) {
        char* empty = (char*)calloc(1, 1);
        return empty;
    }
    size_t n = strlen(water);
    H2O h2o;
    h2o_init(&h2o);
    Collector col;
    collector_init(&col, n);

    pthread_t* threads = (pthread_t*)calloc(n, sizeof(pthread_t));
    TArg* args = (TArg*)calloc(n, sizeof(TArg));
    size_t tcount = 0;

    for (size_t i = 0; i < n; ++i) {
        char c = water[i];
        if (c != 'H' && c != 'O') continue;
        args[tcount].h2o = &h2o;
        args[tcount].col = &col;
        args[tcount].type = c;
        if (c == 'H') {
            pthread_create(&threads[tcount], NULL, hydrogen_thread, &args[tcount]);
        } else {
            pthread_create(&threads[tcount], NULL, oxygen_thread, &args[tcount]);
        }
        tcount++;
    }

    for (size_t i = 0; i < tcount; ++i) {
        pthread_join(threads[i], NULL);
    }

    char* result = (char*)calloc(col.pos + 1, 1);
    if (result) {
        memcpy(result, col.buf, col.pos);
        result[col.pos] = '\0';
    }

    free(threads);
    free(args);
    collector_destroy(&col);
    h2o_destroy(&h2o);
    return result;
}

static int is_valid_water(const char* s) {
    if (!s) return 0;
    size_t n = strlen(s);
    if (n % 3 != 0) return 0;
    for (size_t i = 0; i < n; i += 3) {
        int h = 0, o = 0;
        for (int j = 0; j < 3; ++j) {
            char c = s[i + j];
            if (c == 'H') h++;
            else if (c == 'O') o++;
            else return 0;
        }
        if (h != 2 || o != 1) return 0;
    }
    return 1;
}

int main(void) {
    const char* tests[5] = {
        "HOH",
        "OOHHHH",
        "HHOOHH",
        "OHOHHH",
        "HHOHHO"
    };
    for (int i = 0; i < 5; ++i) {
        char* out = build_water(tests[i]);
        printf("in=%s out=%s valid=%s\n", tests[i], out, is_valid_water(out) ? "true" : "false");
        free(out);
    }
    return 0;
}