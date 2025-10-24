#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int n;
    int turn; // 0 for foo, 1 for bar
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    char* buffer;
    size_t idx;
} FooBar;

void* foo_thread(void* arg) {
    FooBar* fb = (FooBar*)arg;
    for (int i = 0; i < fb->n; ++i) {
        pthread_mutex_lock(&fb->mutex);
        while (fb->turn != 0) {
            pthread_cond_wait(&fb->cond, &fb->mutex);
        }
        fb->buffer[fb->idx++] = 'f';
        fb->buffer[fb->idx++] = 'o';
        fb->buffer[fb->idx++] = 'o';
        fb->turn = 1;
        pthread_cond_signal(&fb->cond);
        pthread_mutex_unlock(&fb->mutex);
    }
    return NULL;
}

void* bar_thread(void* arg) {
    FooBar* fb = (FooBar*)arg;
    for (int i = 0; i < fb->n; ++i) {
        pthread_mutex_lock(&fb->mutex);
        while (fb->turn != 1) {
            pthread_cond_wait(&fb->cond, &fb->mutex);
        }
        fb->buffer[fb->idx++] = 'b';
        fb->buffer[fb->idx++] = 'a';
        fb->buffer[fb->idx++] = 'r';
        fb->turn = 0;
        pthread_cond_signal(&fb->cond);
        pthread_mutex_unlock(&fb->mutex);
    }
    return NULL;
}

char* solve(int n) {
    FooBar fb;
    fb.n = n;
    fb.turn = 0; // foo starts
    fb.idx = 0;
    pthread_mutex_init(&fb.mutex, NULL);
    pthread_cond_init(&fb.cond, NULL);
    fb.buffer = (char*)malloc((size_t)(6 * n + 1));
    if (!fb.buffer) return NULL;

    pthread_t t1, t2;
    pthread_create(&t1, NULL, foo_thread, &fb);
    pthread_create(&t2, NULL, bar_thread, &fb);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    fb.buffer[fb.idx] = '\0';

    pthread_mutex_destroy(&fb.mutex);
    pthread_cond_destroy(&fb.cond);

    return fb.buffer; // caller must free
}

int main() {
    int tests[5] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; ++i) {
        char* out = solve(tests[i]);
        if (out) {
            printf("%s\n", out);
            free(out);
        }
    }
    return 0;
}