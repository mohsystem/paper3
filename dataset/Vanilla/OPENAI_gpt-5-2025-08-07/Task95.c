#include <stdio.h>
#include <time.h>

double getProcessorTimeSeconds(void) {
    return (double)clock() / (double)CLOCKS_PER_SEC;
}

volatile unsigned long sink_c = 0;
unsigned long burnCpu(unsigned long n) {
    unsigned long s = 0;
    for (unsigned long i = 0; i < n; ++i) {
        s += i % 7;
    }
    sink_c = s;
    return s;
}

int main(void) {
    for (int i = 1; i <= 5; ++i) {
        double secs = getProcessorTimeSeconds();
        printf("Run %d - CPU time (s): %.9f\n", i, secs);
        burnCpu(50000000UL);
    }
    return 0;
}