
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define ARRAY_SIZE 7

void printElementAtIndex(const char* array[], int arraySize, const char* indexStr) {
    if (indexStr == NULL || strlen(indexStr) == 0) {
        printf("Error: Index argument is empty or null\\n");
        return;
    }
    
    char* endptr;
    errno = 0;
    long index = strtol(indexStr, &endptr, 10);
    
    if (errno == ERANGE || index > INT_MAX || index < INT_MIN) {
        printf("Error: Number out of range: %s\\n", indexStr);
        return;
    }
    
    if (*endptr != '\\0') {
        printf("Error: Invalid integer format: %s\\n", indexStr);
        return;
    }
    
    if (index < 0 || index >= arraySize) {
        printf("Error: Index %ld is out of bounds. Valid range: 0-%d\\n", index, arraySize - 1);
        return;
    }
    
    printf("Element at index %ld: %s\\n", index, array[index]);
}

int main() {
    const char* elements[ARRAY_SIZE] = {"apple", "banana", "cherry", "date", "elderberry", "fig", "grape"};
    
    // Test cases
    printf("Test Case 1: Valid index 0\\n");
    printElementAtIndex(elements, ARRAY_SIZE, "0");
    
    printf("\\nTest Case 2: Valid index 3\\n");
    printElementAtIndex(elements, ARRAY_SIZE, "3");
    
    printf("\\nTest Case 3: Negative index\\n");
    printElementAtIndex(elements, ARRAY_SIZE, "-1");
    
    printf("\\nTest Case 4: Out of bounds index\\n");
    printElementAtIndex(elements, ARRAY_SIZE, "10");
    
    printf("\\nTest Case 5: Invalid format\\n");
    printElementAtIndex(elements, ARRAY_SIZE, "abc");
    
    return 0;
}
