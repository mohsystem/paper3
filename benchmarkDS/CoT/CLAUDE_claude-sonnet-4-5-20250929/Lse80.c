
#include <stdio.h>
#include <string.h>

int scanString(char* buffer, int maxSize) {
    if (buffer == NULL || maxSize <= 0) {
        return -1;
    }
    
    /* Clear buffer */
    memset(buffer, 0, maxSize);
    
    /* Use fgets for safe input - reads at most maxSize-1 characters */
    if (fgets(buffer, maxSize, stdin) == NULL) {
        return -1;
    }
    
    /* Remove newline if present */
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\\n') {
        buffer[len - 1] = '\\0';
        len--;
    }
    
    return (int)len;
}

int main() {
    const int BUFFER_SIZE = 1024;
    
    printf("Test Case 1: Short string\\n");
    char buffer1[BUFFER_SIZE];
    printf("Enter input: ");
    int count1 = scanString(buffer1, BUFFER_SIZE);
    printf("Characters read: %d\\n", count1);
    printf("Buffer content: %s\\n", buffer1);
    
    printf("\\nTest Case 2: Empty string\\n");
    char buffer2[BUFFER_SIZE];
    printf("Enter input: ");
    int count2 = scanString(buffer2, BUFFER_SIZE);
    printf("Characters read: %d\\n", count2);
    
    printf("\\nTest Case 3: String with spaces\\n");
    char buffer3[BUFFER_SIZE];
    printf("Enter input: ");
    int count3 = scanString(buffer3, BUFFER_SIZE);
    printf("Characters read: %d\\n", count3);
    printf("Buffer content: %s\\n", buffer3);
    
    printf("\\nTest Case 4: Maximum length string\\n");
    char buffer4[BUFFER_SIZE];
    printf("Enter input: ");
    int count4 = scanString(buffer4, BUFFER_SIZE);
    printf("Characters read: %d\\n", count4);
    
    printf("\\nTest Case 5: Special characters\\n");
    char buffer5[BUFFER_SIZE];
    printf("Enter input: ");
    int count5 = scanString(buffer5, BUFFER_SIZE);
    printf("Characters read: %d\\n", count5);
    printf("Buffer content: %s\\n", buffer5);
    
    return 0;
}
