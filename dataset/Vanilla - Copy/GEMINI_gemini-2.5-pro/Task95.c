#include <stdio.h>
#include <time.h>
#include <math.h>

/**
 * @brief Returns the processor time used by the program since its start.
 * 
 * @return The processor time in seconds as a double.
 */
double getProcessorTimeInSeconds() {
    return (double)clock() / CLOCKS_PER_SEC;
}

int main() {
    printf("C Processor Time Test Cases:\n");

    // Test Case 1: Initial time
    printf("Test 1: %.9f seconds\n", getProcessorTimeInSeconds());

    // Do some work to consume CPU time
    for (long i = 0; i < 200000000L; ++i) {
        // busy loop
    }

    // Test Case 2: Time after some work
    printf("Test 2: %.9f seconds\n", getProcessorTimeInSeconds());

    // Do more intensive work
    double sum = 0;
    for (int i = 0; i < 10000000; ++i) {
        sum += sqrt((double)i);
    }

    // Test Case 3: Time after more work
    printf("Test 3: %.9f seconds\n", getProcessorTimeInSeconds());

    // Test Case 4: Time immediately after, should be very similar to Test 3
    printf("Test 4: %.9f seconds\n", getProcessorTimeInSeconds());

    // Do some final work (calculating factorial to consume CPU)
    long double factorial = 1.0;
    for (int i = 1; i <= 50; i++) {
        factorial *= i;
    }
    
    // Test Case 5: Final time
    printf("Test 5: %.9f seconds\n", getProcessorTimeInSeconds());

    return 0;
}