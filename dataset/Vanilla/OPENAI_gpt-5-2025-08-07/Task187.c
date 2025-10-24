#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

typedef struct {
    sem_t hSlots;
    sem_t oSlots;

    sem_t mutex;       // binary semaphore for count
    sem_t turnstile;   // barrier gate 1
    sem_t turnstile2;  // barrier gate 2
    int count;

    // output buffer
    char* out;
    int out_len;
    int out_pos;
    pthread_mutex_t out_lock;
} H2O;

void h2o_init(H2O* w, int total_len) {
    sem_init(&w->hSlots, 0, 2);
    sem_init(&w->oSlots, 0, 1);
    sem_init(&w->mutex, 0, 1);
    sem_init(&w->turnstile, 0, 0);
    sem_init(&w->turnstile2, 0, 1);
    w->count = 0;
    w->out_len = total_len;
    w->out_pos = 0;
    w->out = (char*)malloc((total_len + 1) * sizeof(char));
    pthread_mutex_init(&w->out_lock, NULL);
}

void h2o_destroy(H2O* w) {
    sem_destroy(&w->hSlots);
    sem_destroy(&w->oSlots);
    sem_destroy(&w->mutex);
    sem_destroy(&w->turnstile);
    sem_destroy(&w->turnstile2);
    pthread_mutex_destroy(&w->out_lock);
    // out freed by caller after retrieval
}

int barrier_phase1(H2O* w) {
    int leader = 0;
    sem_wait(&w->mutex);
    w->count += 1;
    if (w->count == 3) {
        sem_wait(&w->turnstile2);
        for (int i = 0; i < 3; ++i) sem_post(&w->turnstile);
        leader = 1;
    }
    sem_post(&w->mutex);

    sem_wait(&w->turnstile);
    sem_post(&w->turnstile);
    return leader;
}

void barrier_phase2(H2O* w) {
    sem_wait(&w->mutex);
    w->count -= 1;
    if (w->count == 0) {
        sem_wait(&w->turnstile);
        for (int i = 0; i < 3; ++i) sem_post(&w->turnstile2);
    }
    sem_post(&w->mutex);

    sem_wait(&w->turnstile2);
    sem_post(&w->turnstile2);
}

typedef struct {
    H2O* w;
    char type; // 'H' or 'O'
} ThreadArg;

void releaseH(H2O* w) {
    pthread_mutex_lock(&w->out_lock);
    if (w->out_pos < w->out_len) {
        w->out[w->out_pos++] = 'H';
    }
    pthread_mutex_unlock(&w->out_lock);
}

void releaseO(H2O* w) {
    pthread_mutex_lock(&w->out_lock);
    if (w->out_pos < w->out_len) {
        w->out[w->out_pos++] = 'O';
    }
    pthread_mutex_unlock(&w->out_lock);
}

void* hydrogen_thread(void* arg) {
    ThreadArg* ta = (ThreadArg*)arg;
    H2O* w = ta->w;
    sem_wait(&w->hSlots);
    int leader = barrier_phase1(w);
    releaseH(w);
    barrier_phase2(w);
    if (leader) {
        sem_post(&w->hSlots);
        sem_post(&w->hSlots);
        sem_post(&w->oSlots);
    }
    return NULL;
}

void* oxygen_thread(void* arg) {
    ThreadArg* ta = (ThreadArg*)arg;
    H2O* w = ta->w;
    sem_wait(&w->oSlots);
    int leader = barrier_phase1(w);
    releaseO(w);
    barrier_phase2(w);
    if (leader) {
        sem_post(&w->hSlots);
        sem_post(&w->hSlots);
        sem_post(&w->oSlots);
    }
    return NULL;
}

char* make_water(const char* water) {
    int n = (int)strlen(water);
    H2O w;
    h2o_init(&w, n);

    pthread_t* threads = (pthread_t*)malloc(n * sizeof(pthread_t));
    ThreadArg* args = (ThreadArg*)malloc(n * sizeof(ThreadArg));

    for (int i = 0; i < n; ++i) {
        args[i].w = &w;
        args[i].type = water[i];
        if (water[i] == 'H') {
            pthread_create(&threads[i], NULL, hydrogen_thread, &args[i]);
        } else {
            pthread_create(&threads[i], NULL, oxygen_thread, &args[i]);
        }
    }

    for (int i = 0; i < n; ++i) pthread_join(threads[i], NULL);

    w.out[w.out_pos] = '\0';

    free(threads);
    free(args);

    char* result = w.out; // transfer ownership to caller
    h2o_destroy(&w);
    return result;
}

int main() {
    const char* tests[5] = {
        "HOH",
        "OOHHHH",
        "HOHOHO",
        "HHOHHO",
        "OOOHHHHHH"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = make_water(tests[i]);
        printf("Input: %s -> Output: %s\n", tests[i], res);
        free(res);
    }
    return 0;
}