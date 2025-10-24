#include <stdio.h>
#include <time.h>

/**
 * @brief Gets the processor time used by the program since it started.
 * This is measured in clock ticks, which are converted to seconds.
 * @return The processor time in seconds as a double.
 */
double getProcessorTimeInSeconds() {
    // clock() returns the number of clock ticks elapsed since the program was launched.
    // CLOCKS_PER_SEC is the number of clock ticks per second.
    return (double)clock() / CLOCKS_PER_SEC;
}

/**
 * @brief A simple function to consume some CPU time to make the measurements meaningful.
 */
void doSomeWork() {
    long sum = 0;
    // C is fast, so a larger loop is used to see a time difference.
    for (long i = 0; i < 200000000L; ++i) {
        sum += i;
    }
}


int main() {
    printf("Running 5 test cases to measure processor time...\n");

    // Test Case 1: Initial measurement
    printf("Test Case 1: Start time: %.6f seconds\n", getProcessorTimeInSeconds());

    // Test Case 2: After some work
    doSomeWork();
    printf("Test Case 2: After first work: %.6f seconds\n", getProcessorTimeInSeconds());

    // Test Case 3: After more work
    doSomeWork();
    printf("Test Case 3: After second work: %.6f seconds\n", getProcessorTimeInSeconds());

    // Test Case 4: A measurement immediately after the previous one
    printf("Test Case 4: Immediately after: %.6f seconds\n", getProcessorTimeInSeconds());

    // Test Case 5: After a final round of work
    doSomeWork();
    printf("Test Case 5: After final work: %.6f seconds\n", getProcessorTimeInSeconds());

    return 0;
}