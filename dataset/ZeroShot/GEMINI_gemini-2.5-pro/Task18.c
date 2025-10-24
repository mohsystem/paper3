#include <stdio.h>
#include <stddef.h> // For size_t
#include <limits.h> // For LLONG_MAX

/**
 * Converts an array of binary digits (0s and 1s) to its integer equivalent.
 *
 * @param binary Pointer to the start of the integer array.
 * @param size The number of elements in the array.
 * @param error Pointer to an integer where an error code will be stored.
 *              0 for success, 1 for invalid input, 2 for overflow.
 * @return The long long representation of the binary value. Returns 0 on error.
 */
long long binaryArrayToInteger(const int* binary, size_t size, int* error) {
    if (binary == NULL || error == NULL) {
        if (error != NULL) *error = 1; // Invalid argument
        return 0;
    }

    *error = 0; // Assume success
    long long result = 0;
    
    for (size_t i = 0; i < size; ++i) {
        int bit = binary[i];
        if (bit != 0 && bit != 1) {
            *error = 1; // Invalid bit value
            return 0;
        }
        
        // Check for potential overflow before left shift and addition
        if (result > (LLONG_MAX - bit) / 2) {
            *error = 2; // Overflow
            return 0;
        }
        
        result = (result << 1) | bit;
    }
    
    return result;
}

void print_array(const int* arr, size_t size) {
    printf("[");
    for (size_t i = 0; i < size; ++i) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]");
}

void run_test_case(const int* arr, size_t size) {
    printf("Testing: ");
    print_array(arr, size);
    
    int error_code = 0;
    long long result = binaryArrayToInteger(arr, size, &error_code);
    
    switch (error_code) {
        case 0:
            printf(" ==> %lld\n", result);
            break;
        case 1:
            printf(" ==> Error: Invalid input array.\n");
            break;
        case 2:
            printf(" ==> Error: Overflow occurred.\n");
            break;
        default:
            printf(" ==> Error: Unknown error.\n");
            break;
    }
}

int main() {
    // 5 Test cases
    int test1[] = {0, 0, 0, 1};
    run_test_case(test1, sizeof(test1) / sizeof(test1[0]));

    int test2[] = {0, 0, 1, 0};
    run_test_case(test2, sizeof(test2) / sizeof(test2[0]));

    int test3[] = {0, 1, 0, 1};
    run_test_case(test3, sizeof(test3) / sizeof(test3[0]));

    int test4[] = {1, 0, 0, 1};
    run_test_case(test4, sizeof(test4) / sizeof(test4[0]));

    int test5[] = {1, 1, 1, 1};
    run_test_case(test5, sizeof(test5) / sizeof(test5[0]));

    return 0;
}