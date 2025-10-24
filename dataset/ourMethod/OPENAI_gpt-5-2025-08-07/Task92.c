#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_THREADS_CAP 128

typedef struct {
    int currentCounter;
    int maxCounter;
    pthread_mutex_t mtx;
} SharedState;

typedef struct {
    SharedState* shared;
    int workerId;
} WorkerArg;

static void* worker(void* arg) {
    WorkerArg* wa = (WorkerArg*)arg;
    if (wa == NULL || wa->shared == NULL) {
        return NULL;
    }
    SharedState* shared = wa->shared;
    int id = wa->workerId;

    for (;;) {
        if (pthread_mutex_lock(&shared->mtx) != 0) {
            break; // fail closed
        }
        if (shared->currentCounter <= shared->maxCounter) {
            shared->currentCounter += 1;
            // Safe fixed format string with bounded integers
            printf("[Worker-%d] incremented currentCounter to %d (max %d)\n",
                   id, shared->currentCounter, shared->maxCounter);
            fflush(stdout);
            if (pthread_mutex_unlock(&shared->mtx) != 0) {
                break;
            }
        } else {
            pthread_mutex_unlock(&shared->mtx);
            break;
        }
        // yield hint (no portable pthread_yield), continue loop
    }
    return NULL;
}

int run_counter(int start, int max_value, int thread_count) {
    int threads = thread_count;
    if (threads < 1) {
        return start;
    }
    if (threads > MAX_THREADS_CAP) {
        threads = MAX_THREADS_CAP;
    }

    SharedState shared;
    shared.currentCounter = start;
    shared.maxCounter = max_value;
    if (pthread_mutex_init(&shared.mtx, NULL) != 0) {
        return start; // fail closed
    }

    pthread_t* tids = (pthread_t*)calloc((size_t)threads, sizeof(pthread_t));
    WorkerArg* args = (WorkerArg*)calloc((size_t)threads, sizeof(WorkerArg));
    if (tids == NULL || args == NULL) {
        free(tids);
        free(args);
        pthread_mutex_destroy(&shared.mtx);
        return start; // fail closed
    }

    for (int i = 0; i < threads; ++i) {
        args[i].shared = &shared;
        args[i].workerId = i + 1;
        int rc = pthread_create(&tids[i], NULL, worker, &args[i]);
        if (rc != 0) {
            // Join what has been started
            for (int j = 0; j < i; ++j) {
                pthread_join(tids[j], NULL);
            }
            free(tids);
            free(args);
            pthread_mutex_destroy(&shared.mtx);
            return shared.currentCounter;
        }
    }

    for (int i = 0; i < threads; ++i) {
        pthread_join(tids[i], NULL);
    }

    free(tids);
    free(args);
    pthread_mutex_destroy(&shared.mtx);
    return shared.currentCounter;
}

int main(void) {
    // 5 test cases
    int result1 = run_counter(0, 5, 3);
    printf("Final counter (test1): %d\n", result1);

    int result2 = run_counter(4, 4, 2);
    printf("Final counter (test2): %d\n", result2);

    int result3 = run_counter(10, 5, 4);
    printf("Final counter (test3): %d\n", result3);

    int result4 = run_counter(0, 50, 8);
    printf("Final counter (test4): %d\n", result4);

    int result5 = run_counter(-2, 2, 2);
    printf("Final counter (test5): %d\n", result5);

    return 0;
}