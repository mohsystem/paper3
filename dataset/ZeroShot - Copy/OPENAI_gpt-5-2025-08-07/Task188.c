#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <limits.h>

typedef struct {
    int n;
    int turn; // 0 for foo, 1 for bar
    pthread_mutex_t m;
    pthread_cond_t cv;
    char *out;
    size_t idx;
    size_t cap;
} FooBar;

static void* foo_thread(void* arg) {
    FooBar* fb = (FooBar*)arg;
    for (int i = 0; i < fb->n; ++i) {
        if (pthread_mutex_lock(&fb->m) != 0) return NULL;
        while (fb->turn != 0) {
            pthread_cond_wait(&fb->cv, &fb->m);
        }
        // write "foo"
        if (fb->idx + 3 <= fb->cap) {
            fb->out[fb->idx++] = 'f';
            fb->out[fb->idx++] = 'o';
            fb->out[fb->idx++] = 'o';
        }
        fb->turn = 1;
        pthread_cond_signal(&fb->cv);
        pthread_mutex_unlock(&fb->m);
    }
    return NULL;
}

static void* bar_thread(void* arg) {
    FooBar* fb = (FooBar*)arg;
    for (int i = 0; i < fb->n; ++i) {
        if (pthread_mutex_lock(&fb->m) != 0) return NULL;
        while (fb->turn != 1) {
            pthread_cond_wait(&fb->cv, &fb->m);
        }
        // write "bar"
        if (fb->idx + 3 <= fb->cap) {
            fb->out[fb->idx++] = 'b';
            fb->out[fb->idx++] = 'a';
            fb->out[fb->idx++] = 'r';
        }
        fb->turn = 0;
        pthread_cond_signal(&fb->cv);
        pthread_mutex_unlock(&fb->m);
    }
    return NULL;
}

// Run the FooBar threading task and return a heap-allocated string result.
// Caller must free the returned pointer.
char* run_foobar(int n) {
    if (n < 1 || n > 1000) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    size_t needed = (size_t)6 * (size_t)n + 1; // "foobar" is 6 chars
    if (needed < (size_t)6 * (size_t)n) { // overflow check
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    FooBar fb;
    fb.n = n;
    fb.turn = 0;
    fb.idx = 0;
    fb.cap = needed - 1;
    fb.out = (char*)malloc(needed);
    if (!fb.out) {
        return NULL;
    }
    fb.out[0] = '\0';

    if (pthread_mutex_init(&fb.m, NULL) != 0) {
        free(fb.out);
        return NULL;
    }
    if (pthread_cond_init(&fb.cv, NULL) != 0) {
        pthread_mutex_destroy(&fb.m);
        free(fb.out);
        return NULL;
    }

    pthread_t t1, t2;
    if (pthread_create(&t1, NULL, foo_thread, &fb) != 0) {
        pthread_cond_destroy(&fb.cv);
        pthread_mutex_destroy(&fb.m);
        free(fb.out);
        return NULL;
    }
    if (pthread_create(&t2, NULL, bar_thread, &fb) != 0) {
        pthread_join(t1, NULL);
        pthread_cond_destroy(&fb.cv);
        pthread_mutex_destroy(&fb.m);
        free(fb.out);
        return NULL;
    }

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // Null-terminate
    if (fb.idx < fb.cap + 1) {
        fb.out[fb.idx] = '\0';
    } else {
        fb.out[fb.cap] = '\0';
    }

    pthread_cond_destroy(&fb.cv);
    pthread_mutex_destroy(&fb.m);

    return fb.out; // caller must free
}

int main(void) {
    int tests[5] = {1, 2, 3, 5, 10};
    for (int i = 0; i < 5; ++i) {
        char* result = run_foobar(tests[i]);
        if (result) {
            printf("%s\n", result);
            free(result);
        } else {
            printf("\n");
        }
    }
    return 0;
}