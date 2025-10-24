#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <sched.h>

typedef struct {
    pthread_mutex_t mtx;
    atomic_int current;
    int max;
} Shared;

typedef struct {
    Shared* shared;
    int id;
} WorkerArgs;

static void* worker_func(void* arg) {
    WorkerArgs* wa = (WorkerArgs*)arg;
    Shared* s = wa->shared;
    int id = wa->id;

    for (;;) {
        int snapshot = atomic_load(&s->current);
        if (snapshot <= s->max) {
            if (pthread_mutex_lock(&s->mtx) != 0) {
                // If lock fails, abort thread to avoid undefined behavior
                return NULL;
            }
            if (atomic_load(&s->current) <= s->max) {
                int newVal = atomic_fetch_add(&s->current, 1) + 1;
                printf("[C] Thread-%d accessing currentCounter=%d\n", id, newVal);
                fflush(stdout);
                pthread_mutex_unlock(&s->mtx);
            } else {
                pthread_mutex_unlock(&s->mtx);
                break;
            }
        } else {
            break;
        }
        sched_yield();
    }
    return NULL;
}

int run_counter(int max_counter, int num_threads) {
    if (num_threads < 0) num_threads = 0;

    Shared shared;
    if (pthread_mutex_init(&shared.mtx, NULL) != 0) {
        return -1;
    }
    atomic_init(&shared.current, 0);
    shared.max = max_counter;

    pthread_t* threads = NULL;
    WorkerArgs* args = NULL;

    if (num_threads > 0) {
        threads = (pthread_t*)calloc((size_t)num_threads, sizeof(pthread_t));
        args = (WorkerArgs*)calloc((size_t)num_threads, sizeof(WorkerArgs));
        if (!threads || !args) {
            free(threads);
            free(args);
            pthread_mutex_destroy(&shared.mtx);
            return -1;
        }
    }

    for (int i = 0; i < num_threads; ++i) {
        args[i].shared = &shared;
        args[i].id = i + 1;
        if (pthread_create(&threads[i], NULL, worker_func, &args[i]) != 0) {
            // On failure, join already created threads and cleanup
            for (int j = 0; j < i; ++j) {
                pthread_join(threads[j], NULL);
            }
            free(threads);
            free(args);
            pthread_mutex_destroy(&shared.mtx);
            return -1;
        }
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    int result = atomic_load(&shared.current);
    pthread_mutex_destroy(&shared.mtx);
    free(threads);
    free(args);
    return result;
}

int main(void) {
    struct { int max; int threads; } tests[5] = {
        {5, 3},
        {10, 5},
        {0, 2},
        {1, 4},
        {50, 10}
    };

    for (int i = 0; i < 5; ++i) {
        int res = run_counter(tests[i].max, tests[i].threads);
        printf("[C] Test %d final currentCounter=%d (max=%d, threads=%d)\n",
               i + 1, res, tests[i].max, tests[i].threads);
    }
    return 0;
}