#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#ifndef _WIN32
#include <unistd.h>
#include <fcntl.h>
#endif

static const char ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

typedef struct {
    char **data;
    size_t size;
    size_t cap;
} StringSet;

static void set_init(StringSet *s) {
    s->size = 0;
    s->cap = 8;
    s->data = (char **)malloc(s->cap * sizeof(char *));
}

static int set_contains(const StringSet *s, const char *str) {
    for (size_t i = 0; i < s->size; ++i) {
        if (strcmp(s->data[i], str) == 0) return 1;
    }
    return 0;
}

static void set_add(StringSet *s, char *str) {
    if (s->size == s->cap) {
        s->cap *= 2;
        s->data = (char **)realloc(s->data, s->cap * sizeof(char *));
    }
    s->data[s->size++] = str;
}

static unsigned char random_byte() {
#ifndef _WIN32
    unsigned char b;
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0) {
        ssize_t r = read(fd, &b, 1);
        close(fd);
        if (r == 1) return b;
    }
#endif
    static int seeded = 0;
    if (!seeded) {
        seeded = 1;
        srand((unsigned)time(NULL) ^ (unsigned)(uintptr_t)&seeded);
    }
    return (unsigned char)(rand() & 0xFF);
}

char* generate_session_id(size_t length) {
    if (length == 0) return NULL;
    static StringSet issued;
    static int initialized = 0;
    if (!initialized) {
        set_init(&issued);
        initialized = 1;
    }

    const size_t ALPHABET_LEN = sizeof(ALPHABET) - 1;

    for (;;) {
        char *buf = (char *)malloc(length + 1);
        if (!buf) return NULL;
        for (size_t i = 0; i < length; ++i) {
            unsigned char b = random_byte();
            buf[i] = ALPHABET[b % ALPHABET_LEN];
        }
        buf[length] = '\0';
        if (!set_contains(&issued, buf)) {
            set_add(&issued, buf);
            return buf;
        }
        free(buf);
    }
}

int main() {
    size_t lengths[5] = {16, 24, 32, 40, 48};
    for (int i = 0; i < 5; ++i) {
        char *sid = generate_session_id(lengths[i]);
        if (sid) {
            printf("%s\n", sid);
            free(sid);
        } else {
            printf("Error generating session ID\n");
        }
    }
    return 0;
}