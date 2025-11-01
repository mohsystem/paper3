#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Allocates two memory chunks, securely copies content between them.
 * 
 * This function demonstrates secure memory handling in C:
 * 1. It checks for NULL input.
 * 2. It checks the return value of malloc to handle allocation failures.
 * 3. It uses memcpy with a size calculated from the source, preventing buffer overflows.
 * 4. It returns a new memory chunk that the caller is responsible for freeing.
 *
 * @param content The C-string to be copied.
 * @return A pointer to a new heap-allocated string containing the copied content,
 *         or NULL if an error occurred. The caller MUST free this memory.
 */
char* secureMemoryCopy(const char* content) {
    if (content == NULL) {
        return NULL; // Handle null input gracefully
    }

    size_t len = strlen(content);
    // +1 for the null terminator
    size_t bufferSize = len + 1;

    // 1. Allocate the first chunk of memory
    char* source = (char*)malloc(bufferSize);
    if (source == NULL) {
        perror("Failed to allocate memory for source");
        return NULL;
    }

    // 1. Allocate the second chunk of memory
    char* destination = (char*)malloc(bufferSize);
    if (destination == NULL) {
        perror("Failed to allocate memory for destination");
        free(source); // IMPORTANT: Clean up previously allocated memory
        return NULL;
    }

    // Copy the input string into the first chunk
    memcpy(source, content, bufferSize);
    
    // 2. Securely copy from the first chunk to the second
    // This copy is secure because 'destination' was allocated with 'bufferSize'
    // and we are copying exactly 'bufferSize' bytes.
    memcpy(destination, source, bufferSize);

    // The 'source' chunk is an intermediate buffer and is no longer needed.
    free(source);
    source = NULL; // Good practice to nullify pointer after freeing

    // 3. Return the destination chunk. The caller now "owns" this memory.
    return destination;
}

// The concept of a main class name Lse120 is not applicable in C.
int main() {
    const char* testCases[] = {
        "Hello, secure world!",
        "This is a test case with various characters 1@#$%.",
        "Short",
        "", // Empty string test case
        "This is a significantly longer string to ensure that the memory allocation and copying process works correctly with a larger amount of data."
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; i++) {
        printf("Test Case %d:\n", i + 1);
        const char* original = testCases[i];
        printf("Original:    \"%s\"\n", original);
        
        char* copiedContent = secureMemoryCopy(original);
        
        // Always check for NULL return from functions that can fail
        if (copiedContent != NULL) {
            printf("Copied:      \"%s\"\n", copiedContent);
            printf("Content matches: %s\n", strcmp(original, copiedContent) == 0 ? "true" : "false");
            
            // IMPORTANT: The caller must free the memory returned by the function
            // to prevent memory leaks.
            free(copiedContent);
        } else {
            fprintf(stderr, "Copying failed for the test case.\n");
        }
        printf("\n");
    }

    return 0;
}