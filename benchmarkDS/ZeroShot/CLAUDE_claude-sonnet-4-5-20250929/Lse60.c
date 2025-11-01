
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printBuffer(int size) {
    if (size <= 0) {
        printf("Buffer size must be positive\\n");
        return;
    }
    
    // Allocate buffer with specified size
    char* buffer = (char*)calloc(size, sizeof(char));
    if (buffer == NULL) {
        printf("Memory allocation failed\\n");
        return;
    }
    
    // Print buffer contents
    printf("Buffer contents: [");
    for (int i = 0; i < size; i++) {
        printf("%d", buffer[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]\\n");
    
    // Free allocated memory
    free(buffer);
}

int main(int argc, char* argv[]) {
    // Test cases
    printf("Test Case 1:\\n");
    printBuffer(5);
    
    printf("\\nTest Case 2:\\n");
    printBuffer(10);
    
    printf("\\nTest Case 3:\\n");
    printBuffer(1);
    
    printf("\\nTest Case 4:\\n");
    printBuffer(0);
    
    printf("\\nTest Case 5:\\n");
    printBuffer(20);
    
    // Command line argument handling
    if (argc > 1) {
        int size = atoi(argv[1]);
        printf("\\nCommand line argument:\\n");
        printBuffer(size);
    }
    
    return 0;
}
