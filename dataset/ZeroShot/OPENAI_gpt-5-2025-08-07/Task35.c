#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef struct {
    unsigned long long on;
    unsigned long long off;
} Pair;

unsigned long long countPassengers(const Pair* stops, size_t n) {
    if (stops == NULL && n != 0) {
        fprintf(stderr, "stops cannot be NULL when n > 0\n");
        exit(EXIT_FAILURE);
    }
    unsigned long long count = 0ULL;
    for (size_t i = 0; i < n; ++i) {
        unsigned long long on = stops[i].on;
        unsigned long long off = stops[i].off;
        if (i == 0 && off != 0ULL) {
            fprintf(stderr, "At first stop, number of people getting off must be zero\n");
            exit(EXIT_FAILURE);
        }
        if (on > ULLONG_MAX - count) {
            fprintf(stderr, "Overflow while adding passengers\n");
            exit(EXIT_FAILURE);
        }
        unsigned long long afterBoard = count + on;
        if (off > afterBoard) {
            fprintf(stderr, "More people getting off than currently on the bus\n");
            exit(EXIT_FAILURE);
        }
        count = afterBoard - off;
    }
    return count;
}

static void runTest(const Pair* stops, size_t n) {
    unsigned long long res = countPassengers(stops, n);
    printf("Remaining: %llu\n", res);
}

int main(void) {
    Pair t1[] = {{10ULL,0ULL},{3ULL,5ULL},{5ULL,8ULL}};
    Pair t2[] = {{3ULL,0ULL},{9ULL,1ULL},{4ULL,10ULL},{12ULL,2ULL},{6ULL,1ULL},{7ULL,10ULL}};
    Pair t3[] = {{0ULL,0ULL}};
    Pair t4[] = {{5ULL,0ULL},{0ULL,0ULL},{0ULL,0ULL}};
    Pair t5[] = {{1000000000ULL,0ULL},{1000000000ULL,0ULL},{0ULL,1000000000ULL}};

    runTest(t1, sizeof(t1)/sizeof(t1[0]));
    runTest(t2, sizeof(t2)/sizeof(t2[0]));
    runTest(t3, sizeof(t3)/sizeof(t3[0]));
    runTest(t4, sizeof(t4)/sizeof(t4[0]));
    runTest(t5, sizeof(t5)/sizeof(t5[0]));

    return 0;
}