
// WARNING: This code contains a critical security vulnerability (double-free)
// For educational purposes only - DO NOT USE IN PRODUCTION

#include <stdio.h>
#include <stdlib.h>

void demonstrateVulnerability(int bufferSize, int freeCount) {
    // Allocate buffer
    char* buffer = (char*)malloc(bufferSize);
    
    if (buffer == NULL) {
        printf("Memory allocation failed!\\n");
        return;
    }
    
    printf("Buffer allocated with size: %d at address: %p\\n", bufferSize, (void*)buffer);
    
    // VULNERABILITY: Freeing the same buffer multiple times
    for (int i = 0; i < freeCount; i++) {
        free(buffer);
        printf("Buffer freed, iteration: %d\\n", i + 1);
        // WARNING: After first free, subsequent frees cause undefined behavior
        // This can lead to crashes, corruption, or security vulnerabilities
    }
    
    printf("WARNING: Double-free vulnerability demonstrated!\\n");
}

int main() {
    printf("Test Case 1:\\n");
    demonstrateVulnerability(1024, 3);
    
    printf("\\nTest Case 2:\\n");
    demonstrateVulnerability(2048, 5);
    
    printf("\\nTest Case 3:\\n");
    demonstrateVulnerability(512, 2);
    
    printf("\\nTest Case 4:\\n");
    demonstrateVulnerability(4096, 1);
    
    printf("\\nTest Case 5:\\n");
    demonstrateVulnerability(256, 10);
    
    return 0;
}
