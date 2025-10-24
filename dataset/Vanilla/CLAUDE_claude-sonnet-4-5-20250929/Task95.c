
#include <stdio.h>
#include <time.h>
#include <unistd.h>

double getCurrentProcessorTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

int main() {
    // Test case 1
    printf("Test case 1:\\n");
    double time1 = getCurrentProcessorTime();
    printf("Current processor time: %.9f seconds\\n", time1);
    
    // Test case 2
    printf("\\nTest case 2:\\n");
    double time2 = getCurrentProcessorTime();
    printf("Current processor time: %.9f seconds\\n", time2);
    
    // Test case 3 - with some delay
    printf("\\nTest case 3 (after 1 second delay):\\n");
    sleep(1);
    double time3 = getCurrentProcessorTime();
    printf("Current processor time: %.9f seconds\\n", time3);
    
    // Test case 4
    printf("\\nTest case 4:\\n");
    double time4 = getCurrentProcessorTime();
    printf("Current processor time: %.9f seconds\\n", time4);
    printf("Time difference from test 3: %.9f seconds\\n", time4 - time3);
    
    // Test case 5 - with computation
    printf("\\nTest case 5 (after computation):\\n");
    double startTime = getCurrentProcessorTime();
    long long sum = 0;
    for (int i = 0; i < 10000000; i++) {
        sum += i;
    }
    double endTime = getCurrentProcessorTime();
    printf("Start time: %.9f seconds\\n", startTime);
    printf("End time: %.9f seconds\\n", endTime);
    printf("Elapsed time: %.9f seconds\\n", endTime - startTime);
    
    return 0;
}
