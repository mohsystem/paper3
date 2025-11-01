
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

int main(int argc, char* argv[]) {
    /* Validate that we have at least 2 arguments */
    if (argc < 3 || argv == NULL) {
        fprintf(stderr, "Error: Insufficient arguments\\n");
        return 1;
    }
    
    /* Validate argv[2] is not null */
    if (argv[2] == NULL) {
        fprintf(stderr, "Error: Invalid index\\n");
        return 1;
    }
    
    /* Validate the string is not empty and within reasonable length */
    size_t len = strlen(argv[2]);
    if (len == 0 || len > 10) {
        fprintf(stderr, "Error: Invalid index format\\n");
        return 1;
    }
    
    /* Convert string to integer with validation */
    char* endptr = NULL;
    errno = 0;
    long index_long = strtol(argv[2], &endptr, 10);
    
    /* Check for conversion errors */
    if (errno == ERANGE || endptr == argv[2] || *endptr != '\\0') {
        fprintf(stderr, "Error: Invalid number format\\n");
        return 1;
    }
    
    /* Check for valid range - prevent negative and overflow */
    if (index_long < 0 || index_long > INT_MAX) {
        fprintf(stderr, "Error: Index out of range\\n");
        return 1;
    }
    
    int index = (int)index_long;
    
    /* Validate index is within valid range [0, argc-1] */
    /* Prevent out of bounds access */
    if (index >= argc) {
        fprintf(stderr, "Error: Index out of bounds\\n");
        return 1;
    }
    
    /* Safely print the argument at the validated index */
    printf("%s\\n", argv[index]);
    
    return 0;
}
