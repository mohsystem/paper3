#include <stdio.h>
#include <time.h>

/**
 * Gets the processor time used by the program since it began, in seconds.
 *
 * @return The processor time in seconds as a double.
 */
double getProcessorTimeInSeconds() {
    // clock() returns the number of clock ticks elapsed since the program was launched.
    // CLOCKS_PER_SEC is a macro defining the number of clock ticks per second.
    // Casting to double before division ensures floating-point arithmetic.
    return (double)clock() / CLOCKS_PER_SEC;
}

/**
 * A simple function to consume some CPU time.
 */
void consumeCpuTime() {
    // Using volatile to ensure the compiler does not optimize away the loop
    // by determining its result is unused.
    volatile long sum = 0;
    for (int i = 0; i < 100000000; ++i) {
        sum += i;
    }
}

int main() {
    printf("Running 5 test cases for processor time measurement in C...\n");
    for (int i = 1; i <= 5; ++i) {
        double startTime = getProcessorTimeInSeconds();
        printf("Test Case %d (start): %.6f seconds\n", i, startTime);

        // Consume some CPU time to see a change
        consumeCpuTime();

        double endTime = getProcessorTimeInSeconds();
        printf("Test Case %d (end):   %.6f seconds\n", i, endTime);
        printf("-------------------------------------\n");
    }
    return 0;
}