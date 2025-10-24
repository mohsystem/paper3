#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int max_counter;
    int current_counter;
    pthread_mutex_t lock;
} shared_t;

void* worker(void* arg) {
    shared_t* s = (shared_t*)arg;
    for (;;) {
        pthread_mutex_lock(&s->lock);
        if (s->current_counter <= s->max_counter) {
            s->current_counter++;
            printf("C Thread %lu incremented currentCounter to %d\n",
                   (unsigned long)pthread_self(), s->current_counter);
            pthread_mutex_unlock(&s->lock);
        } else {
            pthread_mutex_unlock(&s->lock);
            break;
        }
    }
    return NULL;
}

int run_counter(int max_counter, int num_threads) {
    shared_t shared;
    shared.max_counter = max_counter;
    shared.current_counter = 0;
    pthread_mutex_init(&shared.lock, NULL);

    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * num_threads);
    if (!threads) return -1;

    for (int i = 0; i < num_threads; ++i) {
        pthread_create(&threads[i], NULL, worker, &shared);
    }
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    int result = shared.current_counter;

    pthread_mutex_destroy(&shared.lock);
    free(threads);
    return result;
}

int main() {
    printf("C Final: %d\n", run_counter(5, 3));
    printf("C Final: %d\n", run_counter(0, 2));
    printf("C Final: %d\n", run_counter(10, 5));
    printf("C Final: %d\n", run_counter(3, 10));
    printf("C Final: %d\n", run_counter(15, 4));
    return 0;
}