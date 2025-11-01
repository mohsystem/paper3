
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/* Predefined array of ID sequences */
static const int id_sequence[] = {100, 200, 300, 400, 500};
static const size_t id_sequence_length = sizeof(id_sequence) / sizeof(id_sequence[0]);

/**
 * Returns the value at the specified index in the id_sequence array.
 * Validates index bounds before access to prevent out-of-bounds errors.
 * 
 * @param index The index to retrieve
 * @param result Pointer to store the result (NULL check performed)
 * @return 0 on success, -1 on error
 */
int getIdAtIndex(int index, int* result) {
    /* Pointer validation: ensure result pointer is not NULL */
    if (result == NULL) {
        return -1;
    }
    
    /* Input validation: check if index is within valid bounds */
    /* This prevents array out-of-bounds access (CWE-125) */
    /* Cast to size_t after confirming non-negative to avoid signed/unsigned issues */
    if (index < 0 || (size_t)index >= id_sequence_length) {
        /* Fail closed: return error without exposing internal details */
        return -1;
    }
    
    /* Safe array access after bounds validation */
    *result = id_sequence[index];
    return 0;
}

/**
 * Helper function to run test cases
 */
void testCase(int index, int expected, int shouldSucceed) {
    int result = 0;
    int success = getIdAtIndex(index, &result);
    
    int passed = ((success == 0) == shouldSucceed) && 
                 (!shouldSucceed || result == expected);
    
    printf("Test: index=%d, expected=%s, got=%s - %s\\n",
           index,
           shouldSucceed ? (char[20]){0}, snprintf((char[20]){0}, 20, "%d", expected), (char[20]){0} : "FAIL",
           (success == 0) ? (char[20]){0}, snprintf((char[20]){0}, 20, "%d", result), (char[20]){0} : "FAIL",
           passed ? "PASS" : "FAIL");
}

/**
 * Safe string to integer conversion with overflow checking
 */
int safeStringToInt(const char* str, int* value) {
    char* endptr = NULL;
    long val = 0;
    
    /* Validate input pointer */
    if (str == NULL || value == NULL) {
        return -1;
    }
    
    /* Reset errno before conversion */
    errno = 0;
    
    /* Convert string to long with error checking */
    val = strtol(str, &endptr, 10);
    
    /* Check for conversion errors */
    if (errno == ERANGE || val > INT_MAX || val < INT_MIN) {
        /* Overflow or underflow occurred */
        return -1;
    }
    
    /* Check if any characters were converted */
    if (endptr == str || *endptr != '\\0') {
        /* No conversion or trailing characters */
        return -1;
    }
    
    *value = (int)val;
    return 0;
}

int main(void) {
    char input_buffer[64];
    int index = 0;
    int result = 0;
    
    /* Initialize buffer to zero */
    memset(input_buffer, 0, sizeof(input_buffer));
    
    /* Run test cases */
    printf("Running test cases:\\n");
    
    result = 0;
    if (getIdAtIndex(0, &result) == 0 && result == 100) {
        printf("Test: index=0, expected=100, got=%d - PASS\\n", result);
    } else {
        printf("Test: index=0, expected=100 - FAIL\\n");
    }
    
    result = 0;
    if (getIdAtIndex(2, &result) == 0 && result == 300) {
        printf("Test: index=2, expected=300, got=%d - PASS\\n", result);
    } else {
        printf("Test: index=2, expected=300 - FAIL\\n");
    }
    
    result = 0;
    if (getIdAtIndex(4, &result) == 0 && result == 500) {
        printf("Test: index=4, expected=500, got=%d - PASS\\n", result);
    } else {
        printf("Test: index=4, expected=500 - FAIL\\n");
    }
    
    result = 0;
    if (getIdAtIndex(-1, &result) != 0) {
        printf("Test: index=-1, expected=FAIL, got=FAIL - PASS\\n");
    } else {
        printf("Test: index=-1, expected=FAIL - FAIL\\n");
    }
    
    result = 0;
    if (getIdAtIndex(10, &result) != 0) {
        printf("Test: index=10, expected=FAIL, got=FAIL - PASS\\n");
    } else {
        printf("Test: index=10, expected=FAIL - FAIL\\n");
    }
    
    printf("\\nInteractive mode:\\n");
    printf("Enter an index value (0-%zu): ", id_sequence_length - 1);
    
    /* Use fgets for safe input reading with bounds checking */
    if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
        printf("Error: Failed to read input.\\n");
        return EXIT_FAILURE;
    }
    
    /* Remove newline if present */
    size_t len = strlen(input_buffer);
    if (len > 0 && input_buffer[len - 1] == '\\n') {
        input_buffer[len - 1] = '\\0';
    }
    
    /* Validate and convert input to integer with overflow checking */
    if (safeStringToInt(input_buffer, &index) != 0) {
        /* Generic error message - does not expose internal details */
        printf("Error: Invalid input. Please enter a valid integer.\\n");
        return EXIT_FAILURE;
    }
    
    result = 0;
    
    /* Call function with bounds checking */
    if (getIdAtIndex(index, &result) == 0) {
        printf("Value at index %d: %d\\n", index, result);
    } else {
        /* Generic error message - does not expose internal details */
        printf("Error: Invalid index.\\n");
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
