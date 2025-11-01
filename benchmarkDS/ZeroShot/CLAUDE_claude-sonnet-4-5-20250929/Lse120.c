
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void copyAndPrint(const char* source) {
    if (source == NULL) {
        printf("Source is null\\n");
        return;
    }
    
    // Get length of source string
    size_t length = strlen(source);
    
    // Allocate first chunk of memory
    char* firstChunk = (char*)malloc(length + 1);
    if (firstChunk == NULL) {
        printf("Memory allocation failed\\n");
        return;
    }
    
    // Copy source to first chunk
    strncpy(firstChunk, source, length);
    firstChunk[length] = '\\0';
    
    // Allocate second chunk of memory
    char* secondChunk = (char*)malloc(length + 1);
    if (secondChunk == NULL) {
        printf("Memory allocation failed\\n");
        free(firstChunk);
        return;
    }
    
    // Copy first chunk to second chunk
    strncpy(secondChunk, firstChunk, length);
    secondChunk[length] = '\\0';
    
    // Print contents of second chunk
    printf("%s\\n", secondChunk);
    
    // Free allocated memory
    free(firstChunk);
    free(secondChunk);
}

int main() {
    // Test case 1: Normal string
    printf("Test 1:\\n");
    copyAndPrint("Hello, World!");
    
    // Test case 2: Empty string
    printf("\\nTest 2:\\n");
    copyAndPrint("");
    
    // Test case 3: String with special characters
    printf("\\nTest 3:\\n");
    copyAndPrint("Special @#$%% Characters!");
    
    // Test case 4: Null input
    printf("\\nTest 4:\\n");
    copyAndPrint(NULL);
    
    // Test case 5: Long string
    printf("\\nTest 5:\\n");
    copyAndPrint("This is a longer string with multiple words and spaces");
    
    return 0;
}
