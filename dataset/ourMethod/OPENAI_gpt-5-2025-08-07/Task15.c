#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_FLOORS 1000

typedef struct {
    char **lines;
    size_t count;
} TowerResult;

static void free_tower(TowerResult r) {
    if (r.lines != NULL) {
        for (size_t i = 0; i < r.count; ++i) {
            free(r.lines[i]);
        }
        free(r.lines);
    }
}

TowerResult build_tower(int floors) {
    TowerResult res;
    res.lines = NULL;
    res.count = 0;

    if (floors <= 0 || floors > MAX_FLOORS) {
        return res; // invalid -> empty
    }

    size_t width = (size_t)2 * (size_t)floors - 1U;

    char **lines = (char **)calloc((size_t)floors, sizeof(char *));
    if (lines == NULL) {
        return res; // allocation failure
    }

    for (int i = 0; i < floors; ++i) {
        size_t stars = (size_t)2 * (size_t)(i + 1) - 1U;
        if (width < stars) {
            // defensive check
            for (int k = 0; k < i; ++k) {
                free(lines[k]);
            }
            free(lines);
            return res;
        }
        size_t spaces = (width - stars) / 2U;

        char *line = (char *)malloc(width + 1U);
        if (line == NULL) {
            for (int k = 0; k < i; ++k) {
                free(lines[k]);
            }
            free(lines);
            return res;
        }

        memset(line, ' ', width);
        // fill stars
        for (size_t s = 0; s < stars; ++s) {
            line[spaces + s] = '*';
        }
        line[width] = '\0';
        lines[i] = line;
    }

    res.lines = lines;
    res.count = (size_t)floors;
    return res;
}

static void print_tower(const TowerResult r) {
    if (r.lines == NULL || r.count == 0U) {
        printf("(invalid or empty tower)\n");
        return;
    }
    for (size_t i = 0; i < r.count; ++i) {
        printf("%s\n", r.lines[i]);
    }
}

int main(void) {
    // 5 test cases
    int tests[5] = {1, 2, 3, 6, 0};
    for (size_t i = 0; i < 5U; ++i) {
        int floors = tests[i];
        printf("Floors = %d\n", floors);
        TowerResult tr = build_tower(floors);
        print_tower(tr);
        free_tower(tr);
        printf("----\n");
    }
    return 0;
}