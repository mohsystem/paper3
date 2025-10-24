#include <time.h>
#include <stdio.h>

static volatile unsigned long long G_SINK = 0ULL;

/* Returns CPU time used by the process in seconds, or -1.0 on failure. */
double getProcessorTimeSeconds(void) {
    clock_t c = clock();
    if (c == (clock_t)-1) {
        return -1.0;
    }
    return (double)c / (double)CLOCKS_PER_SEC;
}

void doCpuWork(int n) {
    if (n <= 0) return;
    unsigned long long local = G_SINK;
    for (int i = 0; i < n; ++i) {
        unsigned long long ui = (unsigned long long)((unsigned int)i);
        local += (ui ^ (ui << 1));
    }
    G_SINK = local;
}

int main(void) {
    /* 5 test cases */
    for (int i = 1; i <= 5; ++i) {
        doCpuWork(200000 * i);
        double secs = getProcessorTimeSeconds();
        printf("Test %d: %.6f seconds\n", i, secs);
    }
    return 0;
}