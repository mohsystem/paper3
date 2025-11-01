
#include <stdio.h>
#include <string.h>

void allocateAndFillBuffer(int bufferSize) {
    char buffer[bufferSize];
    memset(buffer, 'A', bufferSize);
}

int main() {
    // Test case 1
    allocateAndFillBuffer(10);
    printf("Test case 1: Buffer size 10 - Completed\\n");
    
    // Test case 2
    allocateAndFillBuffer(50);
    printf("Test case 2: Buffer size 50 - Completed\\n");
    
    // Test case 3
    allocateAndFillBuffer(100);
    printf("Test case 3: Buffer size 100 - Completed\\n");
    
    // Test case 4
    allocateAndFillBuffer(1);
    printf("Test case 4: Buffer size 1 - Completed\\n");
    
    // Test case 5
    allocateAndFillBuffer(500);
    printf("Test case 5: Buffer size 500 - Completed\\n");
    
    return 0;
}
