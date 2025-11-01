
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/* Function to get element at specified index with full validation */
const char* getElementAtIndex(int argc, char* argv[]) {
    /* Predefined array of elements */
    static const char* elements[] = {"apple", "banana", "cherry", "date", "elderberry"};
    const size_t elementsSize = sizeof(elements) / sizeof(elements[0]);
    
    /* Validate that we have at least one argument */
    if (argc < 2 || argv == NULL || argv[1] == NULL) {
        return "Error: No index provided";
    }
    
    /* Validate and parse the index */
    char* endptr = NULL;
    errno = 0;
    
    /* Use strtol for safe conversion with comprehensive error checking */
    long indexLong = strtol(argv[1], &endptr, 10);
    
    /* Check for conversion errors: range error, no conversion, or trailing characters */
    if (errno == ERANGE || endptr == argv[1] || *endptr != '\\0') {
        return "Error: Invalid index format";
    }
    
    /* Check for integer overflow/underflow and negative values */
    if (indexLong < 0 || indexLong > INT_MAX) {
        return "Error: Index out of bounds";
    }
    
    int index = (int)indexLong;
    
    /* Bounds checking - ensure index is within valid array range */
    if (index < 0 || (size_t)index >= elementsSize) {
        return "Error: Index out of bounds";
    }
    
    /* Return the element at the validated index */
    return elements[index];
}

int main(int argc, char* argv[]) {
    /* Test case 1: Valid index 0 */
    char* test1[] = {"program", "0"};
    printf("Test 1 (index 0): %s\\n", getElementAtIndex(2, test1));
    
    /* Test case 2: Valid index 2 */
    char* test2[] = {"program", "2"};
    printf("Test 2 (index 2): %s\\n", getElementAtIndex(2, test2));
    
    /* Test case 3: Valid index 4 (last element) */
    char* test3[] = {"program", "4"};
    printf("Test 3 (index 4): %s\\n", getElementAtIndex(2, test3));
    
    /* Test case 4: Invalid index (out of bounds) */
    char* test4[] = {"program", "10"};
    printf("Test 4 (index 10): %s\\n", getElementAtIndex(2, test4));
    
    /* Test case 5: Invalid input (not a number) */
    char* test5[] = {"program", "abc"};
    printf("Test 5 (invalid): %s\\n", getElementAtIndex(2, test5));
    
    return 0;
}
