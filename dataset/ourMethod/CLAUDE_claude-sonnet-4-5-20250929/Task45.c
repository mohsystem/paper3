
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>

/* Maximum input length to prevent excessive memory allocation */
#define MAX_INPUT_LENGTH 1024
#define MAX_OPERATION_LENGTH 20

/* Safe integer input function with overflow protection */
bool safeReadInteger(int* value) {
    char buffer[32];
    
    /* Initialize buffer to zero */
    memset(buffer, 0, sizeof(buffer));
    
    /* Read input with bounds checking using fgets */
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return false;
    }
    
    /* Remove newline if present */
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\\n') {
        buffer[len - 1] = '\\0';
        len--;
    }
    
    /* Validate input is not empty */
    if (len == 0) {
        return false;
    }
    
    /* Validate input contains only digits and optional leading sign */
    size_t startPos = 0;
    if (buffer[0] == '-' || buffer[0] == '+') {
        startPos = 1;
    }
    
    for (size_t i = startPos; i < len; i++) {
        if (!isdigit((unsigned char)buffer[i])) {
            return false;
        }
    }
    
    /* Convert to long and check for overflow */
    errno = 0;
    char* endptr = NULL;
    long temp = strtol(buffer, &endptr, 10);
    
    /* Check for conversion errors */
    if (errno == ERANGE || endptr == buffer || *endptr != '\\0') {
        return false;
    }
    
    /* Check for overflow against int limits */
    if (temp > INT_MAX || temp < INT_MIN) {
        return false;
    }
    
    if (value != NULL) {
        *value = (int)temp;
    }
    return true;
}

/* Safe string input with length validation */
bool safeReadString(char* str, size_t maxLength) {
    /* Validate parameters */
    if (str == NULL || maxLength == 0) {
        return false;
    }
    
    /* Initialize buffer to zero */
    memset(str, 0, maxLength);
    
    /* Read input with bounds checking using fgets */
    if (fgets(str, (int)maxLength, stdin) == NULL) {
        return false;
    }
    
    /* Remove newline if present */
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\\n') {
        str[len - 1] = '\\0';
    }
    
    /* Validate length */
    if (strlen(str) >= maxLength - 1) {
        str[0] = '\\0';
        return false;
    }
    
    return true;
}

/* Safe division operation with divide-by-zero check */
bool safeDivide(int numerator, int denominator, int* result) {
    /* Validate result pointer */
    if (result == NULL) {
        return false;
    }
    
    /* Check for division by zero */
    if (denominator == 0) {
        return false;
    }
    
    /* Check for integer overflow (INT_MIN / -1 causes overflow) */
    if (numerator == INT_MIN && denominator == -1) {
        return false;
    }
    
    *result = numerator / denominator;
    return true;
}

/* Safe multiplication with overflow detection */
bool safeMultiply(int a, int b, int* result) {
    /* Validate result pointer */
    if (result == NULL) {
        return false;
    }
    
    /* Check for multiplication overflow using division */
    if (a > 0 && b > 0 && a > INT_MAX / b) {
        return false;
    }
    if (a > 0 && b < 0 && b < INT_MIN / a) {
        return false;
    }
    if (a < 0 && b > 0 && a < INT_MIN / b) {
        return false;
    }
    if (a < 0 && b < 0 && a < INT_MAX / b) {
        return false;
    }
    
    *result = a * b;
    return true;
}

/* Safe addition with overflow detection */
bool safeAdd(int a, int b, int* result) {
    /* Validate result pointer */
    if (result == NULL) {
        return false;
    }
    
    /* Check for addition overflow */
    if (b > 0 && a > INT_MAX - b) {
        return false;
    }
    if (b < 0 && a < INT_MIN - b) {
        return false;
    }
    
    *result = a + b;
    return true;
}

/* Process operation with comprehensive error handling */
bool processOperation(const char* operation, int operand1, int operand2, int* result) {
    /* Validate input parameters */
    if (operation == NULL || result == NULL) {
        return false;
    }
    
    /* Validate operation string length */
    size_t opLen = strlen(operation);
    if (opLen == 0 || opLen > MAX_OPERATION_LENGTH) {
        return false;
    }
    
    /* Perform operation based on validated input */
    if (strcmp(operation, "add") == 0) {
        return safeAdd(operand1, operand2, result);
    } else if (strcmp(operation, "multiply") == 0) {
        return safeMultiply(operand1, operand2, result);
    } else if (strcmp(operation, "divide") == 0) {
        return safeDivide(operand1, operand2, result);
    } else {
        return false;
    }
}

int main(void) {
    int result = 0;
    
    /* Test case 1: Valid addition */
    printf("Test 1: Addition (10 + 20)\\n");
    result = 0;
    if (processOperation("add", 10, 20, &result)) {
        printf("Result: %d\\n", result);
    } else {
        printf("Error: Operation failed\\n");
    }
    
    /* Test case 2: Valid multiplication */
    printf("\\nTest 2: Multiplication (5 * 6)\\n");
    result = 0;
    if (processOperation("multiply", 5, 6, &result)) {
        printf("Result: %d\\n", result);
    } else {
        printf("Error: Operation failed\\n");
    }
    
    /* Test case 3: Valid division */
    printf("\\nTest 3: Division (100 / 4)\\n");
    result = 0;
    if (processOperation("divide", 100, 4, &result)) {
        printf("Result: %d\\n", result);
    } else {
        printf("Error: Operation failed\\n");
    }
    
    /* Test case 4: Division by zero error */
    printf("\\nTest 4: Division by zero (10 / 0)\\n");
    result = 0;
    if (processOperation("divide", 10, 0, &result)) {
        printf("Result: %d\\n", result);
    } else {
        printf("Error: Division by zero detected and prevented\\n");
    }
    
    /* Test case 5: Overflow detection */
    printf("\\nTest 5: Integer overflow (INT_MAX + 1)\\n");
    result = 0;
    if (processOperation("add", INT_MAX, 1, &result)) {
        printf("Result: %d\\n", result);
    } else {
        printf("Error: Integer overflow detected and prevented\\n");
    }
    
    return 0;
}
