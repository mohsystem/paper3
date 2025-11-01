
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void memoryAllocationMethod() {
    // Allocate first buffer of 1024 bytes
    char* buf = (char*)malloc(1024 * sizeof(char));
    if (buf == NULL) {
        fprintf(stderr, "Memory allocation failed for buf\\n");
        return;
    }
    memset(buf, 0, 1024);
    
    // Read up to 100 characters from stdin into first buffer
    if (fgets(buf, 101, stdin) == NULL) {
        buf[0] = '\\0';
    } else {
        // Remove newline if present
        size_t len = strlen(buf);
        if (len > 0 && buf[len-1] == '\\n') {
            buf[len-1] = '\\0';
        }
    }
    
    // Allocate second buffer of 1024 bytes
    char* buf1 = (char*)malloc(1024 * sizeof(char));
    if (buf1 == NULL) {
        fprintf(stderr, "Memory allocation failed for buf1\\n");
        free(buf);
        return;
    }
    memset(buf1, 0, 1024);
    
    // Read up to 100 characters from stdin into second buffer
    if (fgets(buf1, 101, stdin) == NULL) {
        buf1[0] = '\\0';
    } else {
        // Remove newline if present
        size_t len = strlen(buf1);
        if (len > 0 && buf1[len-1] == '\\n') {
            buf1[len-1] = '\\0';
        }
    }
    
    printf("First buffer (first 100 chars): %s\\n", buf);
    printf("Second buffer (first 100 chars): %s\\n", buf1);
    
    // Free allocated memory
    free(buf);
    free(buf1);
}

int main() {
    printf("Test Case 1:\\n");
    printf("Enter first input and second input:\\n");
    memoryAllocationMethod();
    
    printf("\\nTest Case 2:\\n");
    printf("Enter first input and second input:\\n");
    memoryAllocationMethod();
    
    printf("\\nTest Case 3:\\n");
    printf("Enter first input and second input:\\n");
    memoryAllocationMethod();
    
    printf("\\nTest Case 4:\\n");
    printf("Enter first input and second input:\\n");
    memoryAllocationMethod();
    
    printf("\\nTest Case 5:\\n");
    printf("Enter first input and second input:\\n");
    memoryAllocationMethod();
    
    return 0;
}
