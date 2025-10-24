
#include <stdio.h>
#include <time.h>
#include <math.h>

double getProcessorTimeInSeconds() {
    return (double)clock() / CLOCKS_PER_SEC;
}

void printProcessorTime() {
    double processorTime = getProcessorTimeInSeconds();
    printf("Current processor time: %.6f seconds\\n", processorTime);
}

int main() {
    printf("Test Case 1:\\n");
    printProcessorTime();
    
    printf("\\nTest Case 2:\\n");
    for (int i = 0; i < 1000000; i++) {
        double temp = sqrt(i);
    }
    printProcessorTime();
    
    printf("\\nTest Case 3:\\n");
    for (int i = 0; i < 5000000; i++) {
        double temp = sin(i);
    }
    printProcessorTime();
    
    printf("\\nTest Case 4:\\n");
    for (volatile int i = 0; i < 100000000; i++) {
        /* Busy wait to consume CPU time */
    }
    printProcessorTime();
    
    printf("\\nTest Case 5:\\n");
    for (int i = 0; i < 10000000; i++) {
        char buffer[20];
        sprintf(buffer, "%d", i);
    }
    printProcessorTime();
    
    return 0;
}
