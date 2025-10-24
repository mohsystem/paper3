#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int currentCounter;
    int maxCounter;
    pthread_mutex_t lock;
} SharedCounter;

typedef struct {
    SharedCounter* shared;
    int thread_index;
} ThreadArg;

static int increment_if_allowed(SharedCounter* sc, int thread_index) {
    int incremented = 0;
    if (pthread_mutex_lock(&sc->lock) != 0) {
        return 0;
    }
    if (sc->currentCounter <= sc->maxCounter) {
        sc->currentCounter += 1;
        printf("Thread T-%d incremented currentCounter to %d\n", thread_index, sc->currentCounter);
        fflush(stdout);
        incremented = 1;
    }
    pthread_mutex_unlock(&sc->lock);
    return incremented;
}

static void* worker(void* arg) {
    ThreadArg* ta = (ThreadArg*)arg;
    for (;;) {
        if (!increment_if_allowed(ta->shared, ta->thread_index)) {
            break;
        }
        sched_yield();
    }
    return NULL;
}

int run_counter(int maxCounter, int threadCount) {
    if (threadCount <= 0) return 0;

    SharedCounter sc;
    sc.currentCounter = 0;
    sc.maxCounter = maxCounter;
    if (pthread_mutex_init(&sc.lock, NULL) != 0) {
        return 0;
    }

    pthread_t* threads = (pthread_t*)calloc((size_t)threadCount, sizeof(pthread_t));
    ThreadArg* args = (ThreadArg*)calloc((size_t)threadCount, sizeof(ThreadArg));
    if (!threads || !args) {
        pthread_mutex_destroy(&sc.lock);
        free(threads);
        free(args);
        return 0;
    }

    for (int i = 0; i < threadCount; ++i) {
        args[i].shared = &sc;
        args[i].thread_index = i;
        if (pthread_create(&threads[i], NULL, worker, &args[i]) != 0) {
            // If thread creation fails, continue joining created threads
            threadCount = i;
            break;
        }
    }

    for (int i = 0; i < threadCount; ++i) {
        pthread_join(threads[i], NULL);
    }

    int finalCount = sc.currentCounter;
    pthread_mutex_destroy(&sc.lock);
    free(threads);
    free(args);
    return finalCount;
}

int main(void) {
    printf("Final count (max=5, threads=3): %d\n", run_counter(5, 3));
    printf("Final count (max=8, threads=4): %d\n", run_counter(8, 4));
    printf("Final count (max=0, threads=2): %d\n", run_counter(0, 2));
    printf("Final count (max=1, threads=1): %d\n", run_counter(1, 1));
    printf("Final count (max=3, threads=5): %d\n", run_counter(3, 5));
    return 0;
}