
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/**
 * Returns the command line argument at the index specified by argv[1].
 * 
 * @param argc Argument count
 * @param argv Argument vector (array of C-strings)
 * @return The argument at the specified index, or NULL if invalid
 */
const char* get_argument_at_index(int argc, char* argv[]) {
    /* Check if we have at least 2 arguments (argv[0] and argv[1]) */
    if (argv == NULL || argc < 2) {
        return NULL;
    }
    
    /* Validate argv[1] is not null before processing */
    if (argv[1] == NULL) {
        return NULL;
    }
    
    /* Parse the index from argv[1] using strtol for safe conversion */
    char* endptr = NULL;
    errno = 0;
    long index_long = strtol(argv[1], &endptr, 10);
    
    /* Check for conversion errors:
     * - endptr points to argv[1]: no digits were found
     * - *endptr != '\\0': extra characters after number
     * - errno is set: overflow/underflow occurred
     */
    if (endptr == argv[1] || *endptr != '\\0' || errno == ERANGE) {
        return NULL;
    }
    
    /* Validate index fits in int range and is non-negative */
    if (index_long < 0 || index_long > INT_MAX) {
        return NULL;
    }
    
    int index = (int)index_long;
    
    /* Check bounds to prevent out-of-bounds access */
    if (index >= argc) {
        return NULL;
    }
    
    /* Validate the target argument is not null before returning */
    if (argv[index] == NULL) {
        return NULL;
    }
    
    /* Return the value at the validated index */
    return argv[index];
}

int main(void) {
    /* Test case 1: Valid index within bounds */
    char* test1[] = {"program", "2", "hello", "world"};
    const char* result1 = get_argument_at_index(4, test1);
    printf("Test 1: %s\\n", result1 ? result1 : "null");
    
    /* Test case 2: Index out of bounds */
    char* test2[] = {"program", "5", "hello"};
    const char* result2 = get_argument_at_index(3, test2);
    printf("Test 2: %s\\n", result2 ? result2 : "null");
    
    /* Test case 3: Negative index */
    char* test3[] = {"program", "-1", "hello"};
    const char* result3 = get_argument_at_index(3, test3);
    printf("Test 3: %s\\n", result3 ? result3 : "null");
    
    /* Test case 4: Invalid index format */
    char* test4[] = {"program", "abc", "hello"};
    const char* result4 = get_argument_at_index(3, test4);
    printf("Test 4: %s\\n", result4 ? result4 : "null");
    
    /* Test case 5: Insufficient arguments */
    char* test5[] = {"program"};
    const char* result5 = get_argument_at_index(1, test5);
    printf("Test 5: %s\\n", result5 ? result5 : "null");
    
    return 0;
}
