#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct {
    int n;
    int turn; // 0 = foo, 1 = bar
    size_t pos;
    size_t capacity;
    char *buf;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} foobar_t;

static void safe_append(foobar_t *fb, const char *s, size_t len) {
    if (!fb || !s) return;
    // Ensure capacity is sufficient (should be by design)
    if (fb->pos + len < fb->capacity) {
        memcpy(fb->buf + fb->pos, s, len);
        fb->pos += len;
    }
}

static void *foo_worker(void *arg) {
    foobar_t *fb = (foobar_t *)arg;
    for (int i = 0; i < fb->n; ++i) {
        pthread_mutex_lock(&fb->mutex);
        while (fb->turn != 0) {
            pthread_cond_wait(&fb->cond, &fb->mutex);
        }
        safe_append(fb, "foo", 3);
        fb->turn = 1;
        pthread_cond_signal(&fb->cond);
        pthread_mutex_unlock(&fb->mutex);
    }
    return NULL;
}

static void *bar_worker(void *arg) {
    foobar_t *fb = (foobar_t *)arg;
    for (int i = 0; i < fb->n; ++i) {
        pthread_mutex_lock(&fb->mutex);
        while (fb->turn != 1) {
            pthread_cond_wait(&fb->cond, &fb->mutex);
        }
        safe_append(fb, "bar", 3);
        fb->turn = 0;
        pthread_cond_signal(&fb->cond);
        pthread_mutex_unlock(&fb->mutex);
    }
    return NULL;
}

// Returns a newly allocated string on success; caller must free().
// Returns NULL on invalid input or failure.
char *generate_foobar(int n) {
    if (n < 1 || n > 1000) {
        return NULL;
    }

    foobar_t fb;
    fb.n = n;
    fb.turn = 0;
    fb.pos = 0;
    fb.capacity = (size_t)n * 6 + 1; // "foobar" is 6 chars, + null
    fb.buf = (char *)malloc(fb.capacity);
    if (!fb.buf) {
        return NULL;
    }

    if (pthread_mutex_init(&fb.mutex, NULL) != 0) {
        free(fb.buf);
        return NULL;
    }
    if (pthread_cond_init(&fb.cond, NULL) != 0) {
        pthread_mutex_destroy(&fb.mutex);
        free(fb.buf);
        return NULL;
    }

    pthread_t t1, t2;
    if (pthread_create(&t1, NULL, foo_worker, &fb) != 0) {
        pthread_cond_destroy(&fb.cond);
        pthread_mutex_destroy(&fb.mutex);
        free(fb.buf);
        return NULL;
    }
    if (pthread_create(&t2, NULL, bar_worker, &fb) != 0) {
        pthread_join(t1, NULL);
        pthread_cond_destroy(&fb.cond);
        pthread_mutex_destroy(&fb.mutex);
        free(fb.buf);
        return NULL;
    }

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // Null-terminate
    if (fb.pos < fb.capacity) {
        fb.buf[fb.pos] = '\0';
    } else {
        fb.buf[fb.capacity - 1] = '\0';
    }

    pthread_cond_destroy(&fb.cond);
    pthread_mutex_destroy(&fb.mutex);

    // Return the buffer; caller must free
    return fb.buf;
}

int main(void) {
    int tests[5] = {1, 2, 5, 10, 0};
    for (size_t i = 0; i < 5; ++i) {
        int n = tests[i];
        char *res = generate_foobar(n);
        if (res) {
            printf("n=%d -> %s\n", n, res);
            free(res);
        } else {
            printf("n=%d -> error: n must be in [1, 1000]\n", n);
        }
    }
    return 0;
}