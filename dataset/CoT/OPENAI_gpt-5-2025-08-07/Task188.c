/* Task188 - C implementation
   Chain-of-Through process:
   1) Problem understanding: output "foobar" n times using two threads alternating.
   2) Security requirements: bound n, avoid data races with semaphores, ensure proper cleanup.
   3) Secure coding generation: POSIX threads + semaphores for strict alternation.
   4) Code review: join threads, destroy semaphores, ensure buffer bounds, null terminate.
   5) Secure code output: clamp n to [0,1000]; allocate exact buffer size.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct {
    int n;
    sem_t semFoo;
    sem_t semBar;
    char *buf;
    size_t pos;
} FooBar;

void* foo_thread(void* arg) {
    FooBar* fb = (FooBar*)arg;
    for (int i = 0; i < fb->n; ++i) {
        sem_wait(&fb->semFoo);
        memcpy(fb->buf + fb->pos, "foo", 3);
        fb->pos += 3;
        sem_post(&fb->semBar);
    }
    return NULL;
}

void* bar_thread(void* arg) {
    FooBar* fb = (FooBar*)arg;
    for (int i = 0; i < fb->n; ++i) {
        sem_wait(&fb->semBar);
        memcpy(fb->buf + fb->pos, "bar", 3);
        fb->pos += 3;
        sem_post(&fb->semFoo);
    }
    return NULL;
}

/* Function that accepts input as parameter and returns the output (caller must free) */
char* solve(int n) {
    if (n < 0) n = 0;
    if (n > 1000) n = 1000;

    size_t total = (size_t)n * 6;
    char* buffer = (char*)malloc(total + 1);
    if (!buffer) {
        return NULL;
    }

    FooBar fb;
    fb.n = n;
    fb.buf = buffer;
    fb.pos = 0;

    /* Initialize semaphores: foo starts first */
    if (sem_init(&fb.semFoo, 0, 1) != 0) {
        free(buffer);
        return NULL;
    }
    if (sem_init(&fb.semBar, 0, 0) != 0) {
        sem_destroy(&fb.semFoo);
        free(buffer);
        return NULL;
    }

    pthread_t t1, t2;
    if (pthread_create(&t1, NULL, foo_thread, &fb) != 0) {
        sem_destroy(&fb.semFoo);
        sem_destroy(&fb.semBar);
        free(buffer);
        return NULL;
    }
    if (pthread_create(&t2, NULL, bar_thread, &fb) != 0) {
        pthread_join(t1, NULL);
        sem_destroy(&fb.semFoo);
        sem_destroy(&fb.semBar);
        free(buffer);
        return NULL;
    }

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    buffer[fb.pos] = '\0';

    sem_destroy(&fb.semFoo);
    sem_destroy(&fb.semBar);

    return buffer;
}

int main(void) {
    int tests[5] = {1, 2, 3, 5, 7};
    for (int i = 0; i < 5; ++i) {
        char* out = solve(tests[i]);
        if (out) {
            printf("%s\n", out);
            free(out);
        } else {
            printf("Error\n");
        }
    }
    return 0;
}