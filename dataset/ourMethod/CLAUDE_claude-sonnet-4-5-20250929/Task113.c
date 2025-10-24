
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Security: Maximum limits to prevent resource exhaustion
#define MAX_STRING_LENGTH 10000
#define MAX_TOTAL_LENGTH 100000
#define MAX_STRING_COUNT 1000

// Security: Structure to safely manage dynamic string array
typedef struct {
    char** strings;
    size_t count;
    size_t capacity;
} StringArray;

// Security: Initialize StringArray with NULL checks
StringArray* createStringArray(size_t initialCapacity) {
    // Security: Validate capacity is reasonable
    if (initialCapacity == 0 || initialCapacity > MAX_STRING_COUNT) {
        return NULL;
    }
    
    // Security: Allocate and check for NULL
    StringArray* arr = (StringArray*)malloc(sizeof(StringArray));
    if (arr == NULL) {
        return NULL;
    }
    
    // Security: Initialize all fields
    arr->strings = NULL;
    arr->count = 0;
    arr->capacity = initialCapacity;
    
    // Security: Allocate array with overflow check
    if (initialCapacity > SIZE_MAX / sizeof(char*)) {
        free(arr);
        return NULL;
    }
    
    arr->strings = (char**)calloc(initialCapacity, sizeof(char*));
    if (arr->strings == NULL) {
        free(arr);
        return NULL;
    }
    
    return arr;
}

// Security: Free all allocated memory properly
void freeStringArray(StringArray* arr) {
    if (arr == NULL) {
        return;
    }
    
    if (arr->strings != NULL) {
        // Security: Free each individual string
        for (size_t i = 0; i < arr->count; i++) {
            if (arr->strings[i] != NULL) {
                // Security: Clear sensitive data before freeing
                memset(arr->strings[i], 0, strlen(arr->strings[i]));
                free(arr->strings[i]);
                arr->strings[i] = NULL;
            }
        }
        free(arr->strings);
        arr->strings = NULL;
    }
    
    free(arr);
}

// Security: Validate and add string to array with bounds checking
int addString(StringArray* arr, const char* str) {
    // Security: Validate inputs
    if (arr == NULL || str == NULL) {
        return 0;
    }
    
    // Security: Check string length
    size_t len = strlen(str);
    if (len > MAX_STRING_LENGTH) {
        fprintf(stderr, "String exceeds maximum length\\n");
        return 0;
    }
    
    // Security: Check if array is full
    if (arr->count >= arr->capacity) {
        fprintf(stderr, "String array capacity exceeded\\n");
        return 0;
    }
    
    // Security: Allocate memory for string copy with overflow check
    if (len >= SIZE_MAX - 1) {
        return 0;
    }
    
    char* copy = (char*)malloc(len + 1);
    if (copy == NULL) {
        return 0;
    }
    
    // Security: Use safe string copy with explicit null termination
    memcpy(copy, str, len);
    copy[len] = '\\0';
    
    arr->strings[arr->count] = copy;
    arr->count++;
    
    return 1;
}

// Security: Concatenate strings with comprehensive bounds checking
char* concatenateStrings(const StringArray* arr) {
    // Security: Validate input
    if (arr == NULL || arr->strings == NULL) {
        return NULL;
    }
    
    // Security: Handle empty array
    if (arr->count == 0) {
        char* empty = (char*)malloc(1);
        if (empty != NULL) {
            empty[0] = '\\0';
        }
        return empty;
    }
    
    // Security: Calculate total size with overflow detection
    size_t totalSize = 0;
    for (size_t i = 0; i < arr->count; i++) {
        if (arr->strings[i] == NULL) {
            continue;
        }
        
        size_t len = strlen(arr->strings[i]);
        
        // Security: Check for size_t overflow
        if (totalSize > SIZE_MAX - len) {
            fprintf(stderr, "Size overflow detected\\n");
            return NULL;
        }
        
        totalSize += len;
        
        // Security: Enforce maximum total size
        if (totalSize > MAX_TOTAL_LENGTH) {
            fprintf(stderr, "Total size exceeds maximum\\n");
            return NULL;
        }
    }
    
    // Security: Check for allocation size overflow
    if (totalSize >= SIZE_MAX) {
        return NULL;
    }
    
    // Security: Allocate result buffer with null terminator space
    char* result = (char*)malloc(totalSize + 1);
    if (result == NULL) {
        return NULL;
    }
    
    // Security: Initialize buffer
    result[0] = '\\0';
    size_t offset = 0;
    
    // Security: Safe concatenation with bounds checking
    for (size_t i = 0; i < arr->count; i++) {
        if (arr->strings[i] == NULL) {
            continue;
        }
        
        size_t len = strlen(arr->strings[i]);
        
        // Security: Verify we won't overflow destination buffer\n        if (offset > totalSize - len) {\n            free(result);\n            return NULL;\n        }\n        \n        // Security: Safe copy with explicit bounds\n        memcpy(result + offset, arr->strings[i], len);\n        offset += len;\n    }\n    \n    // Security: Ensure null termination\n    result[offset] = '\\0';\n    \n    return result;\n}\n\n// Security: Safe input reading with buffer overflow protection\nchar* readLine(size_t maxLen) {\n    // Security: Validate max length\n    if (maxLen == 0 || maxLen > MAX_STRING_LENGTH) {\n        return NULL;\n    }\n    \n    // Security: Allocate buffer with space for null terminator\n    char* buffer = (char*)malloc(maxLen + 1);\n    if (buffer == NULL) {\n        return NULL;\n    }\n    \n    // Security: Use fgets with explicit size limit\n    if (fgets(buffer, (int)(maxLen + 1), stdin) == NULL) {\n        free(buffer);\n        return NULL;\n    }\n    \n    // Security: Remove trailing newline if present\n    size_t len = strlen(buffer);\n    if (len > 0 && buffer[len - 1] == '\
') {\n        buffer[len - 1] = '\\0';\n    }\n    \n    return buffer;\n}\n\nint main(void) {\n    // Test case 1: Normal concatenation\n    printf("Test 1: ");\n    {\n        StringArray* arr = createStringArray(4);\n        if (arr != NULL) {\n            addString(arr, "Hello");\n            addString(arr, " ");\n            addString(arr, "World");\n            addString(arr, "!");\n            char* result = concatenateStrings(arr);\n            if (result != NULL) {\n                printf("%s\\n", result);\n                free(result);\n            }\n            freeStringArray(arr);\n        }\n    }\n    \n    // Test case 2: Empty array\n    printf("Test 2: ");\n    {\n        StringArray* arr = createStringArray(1);\n        if (arr != NULL) {\n            char* result = concatenateStrings(arr);\n            if (result != NULL) {\n                printf("'%s'\\n", result);\n                free(result);\n            }\n            freeStringArray(arr);\n        }\n    }\n    \n    // Test case 3: Single string\n    printf("Test 3: ");\n    {\n        StringArray* arr = createStringArray(1);\n        if (arr != NULL) {\n            addString(arr, "SingleString");\n            char* result = concatenateStrings(arr);\n            if (result != NULL) {\n                printf("%s\\n", result);\n                free(result);\n            }\n            freeStringArray(arr);\n        }\n    }\n    \n    // Test case 4: Multiple words\n    printf("Test 4: ");\n    {\n        StringArray* arr = createStringArray(7);\n        if (arr != NULL) {\n            addString(arr, "The");\n            addString(arr, " ");\n            addString(arr, "quick");\n            addString(arr, " ");\n            addString(arr, "brown");\n            addString(arr, " ");\n            addString(arr, "fox");\n            char* result = concatenateStrings(arr);\n            if (result != NULL) {\n                printf("%s\\n", result);\n                free(result);\n            }\n            freeStringArray(arr);\n        }\n    }\n    \n    // Test case 5: Special characters\n    printf("Test 5: ");\n    {\n        StringArray* arr = createStringArray(5);\n        if (arr != NULL) {\n            addString(arr, "User@123");\n            addString(arr, ":");\n            addString(arr, "Pass#456");\n            addString(arr, "!");\n            addString(arr, "Special$Chars");\n            char* result = concatenateStrings(arr);\n            if (result != NULL) {\n                printf("%s\\n", result);\n                free(result);\n            }\n            freeStringArray(arr);\n        }\n    }\n    \n    // Interactive user input section\n    printf("\\n--- Interactive Mode ---\\n");\n    printf("Enter number of strings to concatenate (0 to skip): ");\n    \n    int count = 0;\n    // Security: Validate scanf return value and input range\n    if (scanf("%d", &count) != 1) {\n        fprintf(stderr, "Invalid input\\n");\n        return 1;\n    }\n    \n    // Security: Validate count is within bounds\n    if (count < 0 || count > MAX_STRING_COUNT) {\n        fprintf(stderr, "Count must be between 0 and %d\\n", MAX_STRING_COUNT);\n        return 1;\n    }\n    \n    // Security: Clear input buffer\n    int c;\n    while ((c = getchar()) != '\
' && c != EOF);
    
    if (count > 0) {
        StringArray* arr = createStringArray((size_t)count);
        if (arr == NULL) {
            fprintf(stderr, "Failed to create string array\\n");
            return 1;
        }
        
        for (int i = 0; i < count; i++) {
            printf("Enter string %d: ", i + 1);
            
            // Security: Read line with size limit
            char* input = readLine(MAX_STRING_LENGTH);
            if (input == NULL) {
                fprintf(stderr, "Failed to read input\\n");
                freeStringArray(arr);
                return 1;
            }
            
            // Security: Add string with validation
            if (!addString(arr, input)) {
                fprintf(stderr, "Failed to add string\\n");
                free(input);
                freeStringArray(arr);
                return 1;
            }
            
            free(input);
        }
        
        char* result = concatenateStrings(arr);
        if (result != NULL) {
            printf("Concatenated result: %s\\n", result);
            free(result);
        } else {
            fprintf(stderr, "Concatenation failed\\n");
        }
        
        freeStringArray(arr);
    }
    
    return 0;
}
