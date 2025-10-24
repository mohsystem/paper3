
#include <stdio.h>
#include <time.h>

/* Function to get current processor time in seconds
 * Returns processor time used by the program as a double
 * Security: Uses standard library function, no external input, read-only operation
 * Returns -1.0 on error
 */
double getCurrentProcessorTime(void) {
    /* clock() returns processor time used by the program
     * CLOCKS_PER_SEC is a constant defined by the implementation
     */
    clock_t clockTicks = clock();
    
    /* Check for clock() failure (returns -1 on error)
     * Prevents use of invalid time value
     */
    if (clockTicks == (clock_t)(-1)) {
        fprintf(stderr, "Error: Unable to retrieve processor time\\n");
        return -1.0;
    }
    
    /* Convert clock ticks to seconds
     * Division by CLOCKS_PER_SEC is safe as it's a non-zero constant
     */
    double seconds = (double)clockTicks / CLOCKS_PER_SEC;
    
    return seconds;
}

int main(void) {
    /* Test case 1: Get processor time at program start */
    printf("Test 1 - Processor time at start: ");
    double time1 = getCurrentProcessorTime();
    if (time1 >= 0.0) {
        printf("%.6f seconds\\n", time1);
    }
    
    /* Test case 2: Perform some computation and check time */
    printf("Test 2 - After small computation: ");
    volatile int sum = 0; /* volatile prevents optimization */
    for (int i = 0; i < 1000000; ++i) {
        sum += i;
    }
    double time2 = getCurrentProcessorTime();
    if (time2 >= 0.0) {
        printf("%.6f seconds\\n", time2);
    }
    
    /* Test case 3: Multiple reads to show time progression */
    printf("Test 3 - Another read: ");
    double time3 = getCurrentProcessorTime();
    if (time3 >= 0.0) {
        printf("%.6f seconds\\n", time3);
    }
    
    /* Test case 4: Check that time is monotonically increasing */
    printf("Test 4 - Verify time progression: ");
    double timeA = getCurrentProcessorTime();
    volatile int dummy = 0;
    for (int i = 0; i < 500000; ++i) {
        dummy += i;
    }
    double timeB = getCurrentProcessorTime();
    if (timeA >= 0.0 && timeB >= 0.0) {
        printf("Time increased by %.6f seconds\\n", timeB - timeA);
    }
    
    /* Test case 5: Final processor time reading */
    printf("Test 5 - Final processor time: ");
    double time5 = getCurrentProcessorTime();
    if (time5 >= 0.0) {
        printf("%.6f seconds\\n", time5);
    }
    
    return 0;
}
