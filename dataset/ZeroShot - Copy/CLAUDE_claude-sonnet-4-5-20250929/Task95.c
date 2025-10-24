
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
void sleep_ms(int milliseconds) {
    Sleep(milliseconds);
}
#else
#include <unistd.h>
void sleep_ms(int milliseconds) {
    usleep(milliseconds * 1000);
}
#endif

double getCurrentProcessorTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
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
    printf("\\nTest case 3 (after small delay):\\n");
    sleep_ms(100);
    double time3 = getCurrentProcessorTime();
    printf("Current processor time: %.9f seconds\\n", time3);
    
    // Test case 4
    printf("\\nTest case 4:\\n");
    double time4 = getCurrentProcessorTime();
    printf("Current processor time: %.9f seconds\\n", time4);
    printf("Time elapsed since test case 1: %.9f seconds\\n", time4 - time1);
    
    // Test case 5
    printf("\\nTest case 5:\\n");
    double time5 = getCurrentProcessorTime();
    printf("Current processor time: %.9f seconds\\n", time5);
    printf("Time elapsed since test case 4: %.9f seconds\\n", time5 - time4);
    
    return 0;
}
