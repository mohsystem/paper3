/* 
Chain-of-Through Step 1: Problem understanding
- Implement a function to return current processor time in seconds and demonstrate 5 test cases.

Chain-of-Through Step 2: Security requirements
- No external inputs or unsafe operations.

Chain-of-Through Step 3: Secure coding generation
- Use clock() / CLOCKS_PER_SEC for process CPU time; check for (clock_t)-1.

Chain-of-Through Step 4: Code review
- Types and printing handled safely.

Chain-of-Through Step 5: Secure code output
- Final code below.
*/

#include <time.h>
#include <math.h>
#include <stdio.h>

double getProcessorTimeSeconds(void) {
    clock_t c = clock();
    if (c == (clock_t)-1) {
        return -1.0; // indicate unavailability
    }
    return (double)c / (double)CLOCKS_PER_SEC;
}

volatile double c_sink = 0.0;
void burnCpu(long iterations) {
    double x = 0.0;
    for (long i = 1; i <= iterations; ++i) {
        x += sqrt((double)((i % 1000) + 1));
    }
    c_sink = x; // prevent optimization
}

int main(void) {
    for (int t = 1; t <= 5; ++t) {
        burnCpu(200000L * t);
        double seconds = getProcessorTimeSeconds();
        printf("C Test %d - CPU Time (s): %.9f\n", t, seconds);
    }
    return 0;
}