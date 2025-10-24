
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

// Find the unique number in an array where all other numbers are equal
// Input validation: array must have at least 3 elements and valid pointer
// Security: validates pointer and size to prevent null pointer dereference and buffer overflow
double findUniq(const double* arr, size_t size) {
    // Input validation: check for null pointer
    if (arr == NULL) {
        fprintf(stderr, "Error: NULL pointer provided\\n");
        exit(EXIT_FAILURE);
    }
    
    // Input validation: ensure array has at least 3 elements as guaranteed
    if (size < 3) {
        fprintf(stderr, "Error: Array must contain at least 3 elements\\n");
        exit(EXIT_FAILURE);
    }
    
    // Safe to access first three elements after validation
    double first = arr[0];
    double second = arr[1];
    double third = arr[2];
    
    // Use epsilon for floating point comparison to handle precision issues
    const double epsilon = DBL_EPSILON * 100;
    
    // Determine which value appears more than once (the common value)
    double common;
    double unique_val;
    
    if (fabs(first - second) < epsilon) {
        // first == second, so third might be unique
        common = first;
        unique_val = third;
    } else if (fabs(first - third) < epsilon) {
        // first == third, so second is unique
        common = first;
        unique_val = second;
    } else {
        // second == third, so first is unique
        common = second;
        unique_val = first;
    }
    
    // Verify by counting occurrences to ensure correctness
    // Bounds-checked iteration to prevent buffer overflow
    size_t commonCount = 0;
    size_t uniqueCount = 0;
    
    for (size_t i = 0; i < size; ++i) {
        if (fabs(arr[i] - common) < epsilon) {
            ++commonCount;
        } else if (fabs(arr[i] - unique_val) < epsilon) {
            ++uniqueCount;
        }
    }
    
    // Return the value that appears only once
    if (uniqueCount == 1) {
        return unique_val;
    } else if (commonCount == 1) {
        return common;
    }
    
    // Should not reach here with valid input
    fprintf(stderr, "Error: Could not find unique element\\n");
    exit(EXIT_FAILURE);
}

int main(void) {
    // Test case 1: unique in middle
    double test1[] = {1.0, 1.0, 1.0, 2.0, 1.0, 1.0};
    size_t size1 = sizeof(test1) / sizeof(test1[0]);
    printf("Test 1: %f (expected: 2)\\n", findUniq(test1, size1));
    
    // Test case 2: unique decimal
    double test2[] = {0.0, 0.0, 0.55, 0.0, 0.0};
    size_t size2 = sizeof(test2) / sizeof(test2[0]);
    printf("Test 2: %f (expected: 0.55)\\n", findUniq(test2, size2));
    
    // Test case 3: unique at start
    double test3[] = {5.0, 3.0, 3.0, 3.0, 3.0};
    size_t size3 = sizeof(test3) / sizeof(test3[0]);
    printf("Test 3: %f (expected: 5)\\n", findUniq(test3, size3));
    
    // Test case 4: unique at end
    double test4[] = {7.0, 7.0, 7.0, 7.0, 9.0};
    size_t size4 = sizeof(test4) / sizeof(test4[0]);
    printf("Test 4: %f (expected: 9)\\n", findUniq(test4, size4));
    
    // Test case 5: negative numbers
    double test5[] = {-1.0, -1.0, -1.0, -2.0, -1.0};
    size_t size5 = sizeof(test5) / sizeof(test5[0]);
    printf("Test 5: %f (expected: -2)\\n", findUniq(test5, size5));
    
    return 0;
}
