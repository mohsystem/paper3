#include <stdio.h>
#include <string.h>

void overwriteBuffer(int buffer_size) {
    if (buffer_size < 0) {
        printf("overwriteBuffer(size=%d): Invalid size\n", buffer_size);
        return;
    }
    char buffer[buffer_size];
    if (buffer_size > 0) {
       memset(buffer, 'A', buffer_size);
    }

    printf("overwriteBuffer(size=%d): ", buffer_size);
    for (int i = 0; i < buffer_size; i++) {
        putchar(buffer[i]);
    }
    printf("\n");
}

int main() {
    // Test Case 1
    overwriteBuffer(10);
    
    // Test Case 2
    overwriteBuffer(5);
    
    // Test Case 3
    overwriteBuffer(20);
    
    // Test Case 4
    overwriteBuffer(1);
    
    // Test Case 5
    overwriteBuffer(0);

    return 0;
}