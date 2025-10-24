#include <stdio.h>
#include <time.h>

/**
 * @brief Gets the current processor time in seconds.
 * clock() returns the number of clock ticks elapsed since the program was launched.
 * This value is divided by CLOCKS_PER_SEC to get the time in seconds.
 * 
 * @return The processor time in seconds as a double. Returns -1.0 on failure.
 */
double getProcessorTimeInSeconds() {
    clock_t ticks = clock();
    if (ticks == (clock_t)-1) {
        // clock() returns (clock_t)-1 on error.
        return -1.0;
    }
    return (double)ticks / CLOCKS_PER_SEC;
}

/**
 * @brief A simple function to consume some CPU time.
 * The 'volatile' keyword suggests to the compiler not to optimize the loop away.
 */
void consumeCpuTime() {
    volatile long long sum = 0;
    for (int i = 0; i < 200000000; ++i) {
        sum += i;
    }
}

int main() {
    printf("Running 5 test cases for getting processor time:\n");

    for (int i = 1; i <= 5; ++i) {
        printf("\n--- Test Case %d ---\n", i);
        
        double startTime = getProcessorTimeInSeconds();
        if (startTime < 0.0) {
            fprintf(stderr, "Failed to get processor time.\n");
            return 1;
        }
        printf("Initial processor time: %.6f seconds\n", startTime);

        // Consume some CPU time
        consumeCpuTime();

        double endTime = getProcessorTimeInSeconds();
        if (endTime < 0.0) {
            fprintf(stderr, "Failed to get processor time.\n");
            return 1;
        }
        printf("Processor time after work: %.6f seconds\n", endTime);
        printf("Elapsed processor time: %.6f seconds\n", endTime - startTime);
    }

    return 0;
}