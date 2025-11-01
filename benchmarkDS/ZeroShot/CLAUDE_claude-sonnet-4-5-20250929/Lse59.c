
#include <stdio.h>
#include <string.h>

void fillBuffer(int bufferSize) {
    if (bufferSize <= 0) return;
    
    // Allocate buffer on stack
    char buffer[bufferSize];
    
    // Overwrite all values with 'A'
    memset(buffer, 'A', bufferSize);
}

int main() {
    // Test case 1: Small buffer
    fillBuffer(10);
    printf("Test case 1: Buffer size 10 - Completed\\n");
    
    // Test case 2: Medium buffer
    fillBuffer(100);
    printf("Test case 2: Buffer size 100 - Completed\\n");
    
    // Test case 3: Large buffer
    fillBuffer(1000);
    printf("Test case 3: Buffer size 1000 - Completed\\n");
    
    // Test case 4: Single element
    fillBuffer(1);
    printf("Test case 4: Buffer size 1 - Completed\\n");
    
    // Test case 5: Empty buffer
    fillBuffer(0);
    printf("Test case 5: Buffer size 0 - Completed\\n");
    
    return 0;
}
