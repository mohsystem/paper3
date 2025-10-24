
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Function to validate input array and create phone number
// Security: Validates array pointer, size, and integer ranges (0-9)
// Returns dynamically allocated string - caller must free
// Returns NULL on error
char* createPhoneNumber(const int* numbers, size_t size) {
    // Validate input pointer is not NULL
    if (numbers == NULL) {
        fprintf(stderr, "Error: NULL pointer provided\\n");
        return NULL;
    }
    
    // Validate input size - must be exactly 10 integers
    if (size != 10) {
        fprintf(stderr, "Error: Input must contain exactly 10 integers\\n");
        return NULL;
    }
    
    // Validate each integer is in range 0-9
    for (size_t i = 0; i < size; i++) {
        if (numbers[i] < 0 || numbers[i] > 9) {
            fprintf(stderr, "Error: All integers must be between 0 and 9\\n");
            return NULL;
        }
    }
    
    // Allocate memory for phone number string
    // Format: (###) ###-#### plus null terminator = 15 bytes
    const size_t PHONE_LENGTH = 15;
    char* result = (char*)calloc(PHONE_LENGTH, sizeof(char));
    if (result == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    // Use snprintf for safe string formatting with bounds checking
    // snprintf ensures null termination and prevents buffer overflow
    int written = snprintf(result, PHONE_LENGTH, "(%d%d%d) %d%d%d-%d%d%d%d",
                          numbers[0], numbers[1], numbers[2],
                          numbers[3], numbers[4], numbers[5],
                          numbers[6], numbers[7], numbers[8], numbers[9]);
    
    // Verify snprintf succeeded and didn't truncate
    if (written < 0 || written >= (int)PHONE_LENGTH) {
        fprintf(stderr, "Error: String formatting failed\\n");
        free(result);
        return NULL;
    }
    
    return result;
}

int main(void) {
    // Test case 1: Standard example
    printf("Test 1: ");
    int test1[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    char* result1 = createPhoneNumber(test1, 10);
    if (result1 != NULL) {
        printf("%s\\n", result1);
        free(result1);
        result1 = NULL;
    }
    
    // Test case 2: All zeros
    printf("Test 2: ");
    int test2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    char* result2 = createPhoneNumber(test2, 10);
    if (result2 != NULL) {
        printf("%s\\n", result2);
        free(result2);
        result2 = NULL;
    }
    
    // Test case 3: All nines
    printf("Test 3: ");
    int test3[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
    char* result3 = createPhoneNumber(test3, 10);
    if (result3 != NULL) {
        printf("%s\\n", result3);
        free(result3);
        result3 = NULL;
    }
    
    // Test case 4: Invalid - wrong size
    printf("Test 4: ");
    int test4[] = {1, 2, 3, 4, 5};
    char* result4 = createPhoneNumber(test4, 5);
    if (result4 != NULL) {
        printf("%s\\n", result4);
        free(result4);
        result4 = NULL;
    }
    
    // Test case 5: Invalid - out of range value
    printf("Test 5: ");
    int test5[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    char* result5 = createPhoneNumber(test5, 10);
    if (result5 != NULL) {
        printf("%s\\n", result5);
        free(result5);
        result5 = NULL;
    }
    
    return 0;
}
