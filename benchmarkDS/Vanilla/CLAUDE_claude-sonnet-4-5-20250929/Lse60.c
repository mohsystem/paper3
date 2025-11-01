
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void processBuffer(int size) {
    char* buffer = (char*)malloc(size * sizeof(char));
    if (buffer == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        return;
    }
    
    memset(buffer, 0, size);
    
    printf("Buffer contents:\\n");
    for (int i = 0; i < size; i++) {
        printf("%d ", buffer[i]);
        if ((i + 1) % 20 == 0) {
            printf("\\n");
        }
    }
    printf("\\n");
    
    free(buffer);
}

int main(int argc, char* argv[]) {
    // Test cases
    printf("Test Case 1: Buffer size 10\\n");
    processBuffer(10);
    
    printf("\\nTest Case 2: Buffer size 25\\n");
    processBuffer(25);
    
    printf("\\nTest Case 3: Buffer size 50\\n");
    processBuffer(50);
    
    printf("\\nTest Case 4: Buffer size 5\\n");
    processBuffer(5);
    
    printf("\\nTest Case 5: Buffer size 100\\n");
    processBuffer(100);
    
    // If command line argument is provided
    if (argc > 1) {
        int size = atoi(argv[1]);
        printf("\\nCommand line argument: Buffer size %d\\n", size);
        processBuffer(size);
    }
    
    return 0;
}
