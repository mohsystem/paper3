
#include <stdio.h>
#include <string.h>

int secureReadInput(char* buffer, int maxSize) {
    if (buffer == NULL || maxSize <= 0) {
        return -1;
    }
    
    // Read input using fgets which is safer than scanf
    if (fgets(buffer, maxSize, stdin) == NULL) {
        return 0;
    }
    
    // Remove newline character if present
    int len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\\n') {
        buffer[len - 1] = '\\0';
        len--;
    }
    
    return len;
}

int main() {
    printf("Test Case 1: Short input\\n");
    char buffer1[1024];
    printf("Enter text: ");
    int chars1 = secureReadInput(buffer1, 1024);
    printf("Characters read: %d\\n", chars1);
    
    printf("\\nTest Case 2: Medium input\\n");
    char buffer2[1024];
    printf("Enter text: ");
    int chars2 = secureReadInput(buffer2, 1024);
    printf("Characters read: %d\\n", chars2);
    
    printf("\\nTest Case 3: Long input (near limit)\\n");
    char buffer3[1024];
    printf("Enter text: ");
    int chars3 = secureReadInput(buffer3, 1024);
    printf("Characters read: %d\\n", chars3);
    
    printf("\\nTest Case 4: Empty input\\n");
    char buffer4[1024];
    printf("Enter text: ");
    int chars4 = secureReadInput(buffer4, 1024);
    printf("Characters read: %d\\n", chars4);
    
    printf("\\nTest Case 5: Input exceeding buffer size\\n");
    char buffer5[1024];
    printf("Enter text: ");
    int chars5 = secureReadInput(buffer5, 1024);
    printf("Characters read: %d\\n", chars5);
    
    return 0;
}
