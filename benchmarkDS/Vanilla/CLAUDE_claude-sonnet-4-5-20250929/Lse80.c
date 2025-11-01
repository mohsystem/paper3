
#include <stdio.h>
#include <string.h>

int readString(char* buffer, int maxSize) {
    if (fgets(buffer, maxSize, stdin) == NULL) {
        buffer[0] = '\\0';
        return 0;
    }
    
    int length = strlen(buffer);
    
    if (length > 0 && buffer[length - 1] == '\\n') {
        buffer[length - 1] = '\\0';
        length--;
    }
    
    return length;
}

int main() {
    // Test case 1: Normal input
    printf("Test 1 - Enter a string:\\n");
    char buffer1[1024];
    int count1 = readString(buffer1, 1024);
    printf("Characters read: %d\\n", count1);
    printf("String: %s\\n", buffer1);
    
    // Test case 2: Empty input
    printf("\\nTest 2 - Enter empty string:\\n");
    char buffer2[1024];
    int count2 = readString(buffer2, 1024);
    printf("Characters read: %d\\n", count2);
    
    // Test case 3: Short string
    printf("\\nTest 3 - Enter 'Hello':\\n");
    char buffer3[1024];
    int count3 = readString(buffer3, 1024);
    printf("Characters read: %d\\n", count3);
    printf("String: %s\\n", buffer3);
    
    // Test case 4: Long string
    printf("\\nTest 4 - Enter a long string:\\n");
    char buffer4[1024];
    int count4 = readString(buffer4, 1024);
    printf("Characters read: %d\\n", count4);
    printf("String: %s\\n", buffer4);
    
    // Test case 5: String with spaces
    printf("\\nTest 5 - Enter 'Hello World':\\n");
    char buffer5[1024];
    int count5 = readString(buffer5, 1024);
    printf("Characters read: %d\\n", count5);
    printf("String: %s\\n", buffer5);
    
    return 0;
}
