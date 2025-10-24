#include <stdio.h>
#include <time.h>

double getProcessorTimeSeconds(int mode) {
    if (mode != 0 && mode != 1) {
        return -1.0; /* Fail closed with clear error indicator */
    }
    clock_t ticks = clock();
    if (ticks == (clock_t)-1) {
        return -1.0;
    }
    return (double)ticks / (double)CLOCKS_PER_SEC;
}

double burn_cpu(long iterations) {
    if (iterations < 0) {
        iterations = 0;
    }
    double x = 1.0;
    for (long i = 0; i < iterations; i++) {
        x = x * 1.0000001 + 0.0000001;
        if (x > 2.0e9) {
            x = 1.0;
        }
    }
    return x;
}

int main(void) {
    /* 5 test cases */
    printf("%.9f\n", getProcessorTimeSeconds(0));
    double sink = burn_cpu(2000000L);
    printf("%.9f\n", getProcessorTimeSeconds(1));
    sink += burn_cpu(4000000L);
    printf("%.9f\n", getProcessorTimeSeconds(0));
    sink += burn_cpu(1000000L);
    printf("%.9f\n", getProcessorTimeSeconds(1));
    if (sink == -1.0) { printf(""); }
    printf("%.9f\n", getProcessorTimeSeconds(0));
    return 0;
}