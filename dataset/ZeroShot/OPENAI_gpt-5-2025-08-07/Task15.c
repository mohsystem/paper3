#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct {
    char **lines;
    int size;
} Tower;

static void free_tower(Tower t) {
    if (t.lines != NULL) {
        for (int i = 0; i < t.size; i++) {
            free(t.lines[i]);
        }
        free(t.lines);
    }
}

Tower build_tower(int n) {
    Tower t;
    t.lines = NULL;
    t.size = 0;

    if (n <= 0) {
        return t;
    }

    /* Prevent potential overflow and excessively large allocations */
    if (n > (INT_MAX / 2)) {
        return t;
    }

    int width = 2 * n - 1;
    /* Arbitrary upper bound to avoid excessive allocation */
    const int MAX_WIDTH = 1000000;
    if (width <= 0 || width > MAX_WIDTH) {
        return t;
    }

    char **lines = (char **)malloc(sizeof(char *) * (size_t)n);
    if (lines == NULL) {
        return t;
    }

    for (int i = 1; i <= n; i++) {
        char *line = (char *)malloc((size_t)width + 1);
        if (line == NULL) {
            /* Clean up on failure */
            for (int k = 1; k < i; k++) {
                free(lines[k - 1]);
            }
            free(lines);
            return t;
        }
        memset(line, ' ', (size_t)width);
        line[width] = '\0';

        int stars = 2 * i - 1;
        int spaces = (width - stars) / 2;
        for (int s = 0; s < stars; s++) {
            line[spaces + s] = '*';
        }
        lines[i - 1] = line;
    }

    t.lines = lines;
    t.size = n;
    return t;
}

static void print_tower(const Tower *t) {
    if (t == NULL || t->lines == NULL) return;
    for (int i = 0; i < t->size; i++) {
        puts(t->lines[i]);
    }
}

int main(void) {
    int tests[5] = {1, 2, 3, 6, 0};
    for (int i = 0; i < 5; i++) {
        int n = tests[i];
        printf("n = %d\n", n);
        Tower t = build_tower(n);
        print_tower(&t);
        free_tower(t);
        puts("");
    }
    return 0;
}