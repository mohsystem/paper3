#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

struct Tower {
    char** lines;
    int size;
};

static void free_tower(struct Tower t) {
    if (t.lines != NULL) {
        for (int i = 0; i < t.size; ++i) {
            free(t.lines[i]);
        }
        free(t.lines);
    }
}

struct Tower build_tower(int n) {
    struct Tower t;
    t.lines = NULL;
    t.size = 0;

    if (n <= 0) {
        return t;
    }
    /* check for width overflow: width = 2*n - 1 must fit in size_t */
    if ((size_t)n > (SIZE_MAX - 1) / 2) {
        return t;
    }
    size_t width = 2u * (size_t)n - 1u;

    t.lines = (char**)calloc((size_t)n, sizeof(char*));
    if (!t.lines) {
        return t;
    }

    for (int i = 1; i <= n; ++i) {
        size_t stars = 2u * (size_t)i - 1u;
        size_t spaces = (width - stars) / 2u;

        char* line = (char*)malloc(width + 1u);
        if (!line) {
            for (int k = 0; k < i - 1; ++k) {
                free(t.lines[k]);
            }
            free(t.lines);
            t.lines = NULL;
            t.size = 0;
            return t;
        }

        memset(line, ' ', width);
        for (size_t j = 0; j < stars; ++j) {
            line[spaces + j] = '*';
        }
        line[width] = '\0';
        t.lines[i - 1] = line;
    }
    t.size = n;
    return t;
}

static void print_tower(const struct Tower* t) {
    printf("[\n");
    for (int i = 0; i < t->size; ++i) {
        printf("  \"%s\"%s\n", t->lines[i], (i + 1 < t->size) ? "," : "");
    }
    printf("]\n");
}

int main(void) {
    int tests[5] = {1, 2, 3, 5, 6};
    for (int i = 0; i < 5; ++i) {
        int n = tests[i];
        printf("Floors: %d\n", n);
        struct Tower t = build_tower(n);
        print_tower(&t);
        free_tower(t);
        printf("\n");
    }
    return 0;
}