
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Maximum allowed array size to prevent resource exhaustion
#define MAX_ARRAY_SIZE 1000000

// Comparison function for qsort
// Returns: negative if a < b, zero if a == b, positive if a > b
int compare_ints(const void *a, const void *b) {
    // Safe comparison without overflow
    // Cast pointers to int pointers and dereference
    int arg1 = *(const int*)a;
    int arg2 = *(const int*)b;
    
    // Prevent integer overflow in subtraction
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

// Sorts an array of integers in ascending order
// Parameters: arr - input array, size - number of elements, result - output array
// Returns: 0 on success, -1 on error
int sortArrayAscending(const int *arr, size_t size, int **result) {
    // Input validation: null pointer check
    if (arr == NULL || result == NULL) {
        return -1;
    }
    
    // Input validation: size bounds check to prevent overflow and resource exhaustion
    if (size == 0 || size > MAX_ARRAY_SIZE) {
        return -1;
    }
    
    // Input validation: check for potential integer overflow in allocation size
    if (size > SIZE_MAX / sizeof(int)) {
        return -1;
    }
    
    // Allocate memory for result array
    *result = (int*)malloc(size * sizeof(int));
    if (*result == NULL) {
        // Memory allocation failed
        return -1;
    }
    
    // Copy input array to result (bounds are validated above)
    memcpy(*result, arr, size * sizeof(int));
    
    // Sort using standard library qsort (well-tested, secure implementation)
    qsort(*result, size, sizeof(int), compare_ints);
    
    return 0;
}

int main(void) {
    int *result = NULL;
    
    // Test case 1: Normal array
    int test1[] = {64, 34, 25, 12, 22, 11, 90};
    size_t size1 = sizeof(test1) / sizeof(test1[0]);
    if (sortArrayAscending(test1, size1, &result) == 0 && result != NULL) {
        printf("Test 1: ");
        for (size_t i = 0; i < size1; i++) {
            printf("%d ", result[i]);
        }
        printf("\\n");
        free(result);
        result = NULL;
    }
    
    // Test case 2: Already sorted array
    int test2[] = {1, 2, 3, 4, 5};
    size_t size2 = sizeof(test2) / sizeof(test2[0]);
    if (sortArrayAscending(test2, size2, &result) == 0 && result != NULL) {
        printf("Test 2: ");
        for (size_t i = 0; i < size2; i++) {
            printf("%d ", result[i]);
        }
        printf("\\n");
        free(result);
        result = NULL;
    }
    
    // Test case 3: Reverse sorted array
    int test3[] = {9, 7, 5, 3, 1};
    size_t size3 = sizeof(test3) / sizeof(test3[0]);
    if (sortArrayAscending(test3, size3, &result) == 0 && result != NULL) {
        printf("Test 3: ");
        for (size_t i = 0; i < size3; i++) {
            printf("%d ", result[i]);
        }
        printf("\\n");
        free(result);
        result = NULL;
    }
    
    // Test case 4: Array with negative numbers
    int test4[] = {-5, 10, -3, 0, 8, -1};
    size_t size4 = sizeof(test4) / sizeof(test4[0]);
    if (sortArrayAscending(test4, size4, &result) == 0 && result != NULL) {
        printf("Test 4: ");
        for (size_t i = 0; i < size4; i++) {
            printf("%d ", result[i]);
        }
        printf("\\n");
        free(result);
        result = NULL;
    }
    
    // Test case 5: Array with duplicates
    int test5[] = {5, 2, 8, 2, 9, 1, 5, 5};
    size_t size5 = sizeof(test5) / sizeof(test5[0]);
    if (sortArrayAscending(test5, size5, &result) == 0 && result != NULL) {
        printf("Test 5: ");
        for (size_t i = 0; i < size5; i++) {
            printf("%d ", result[i]);
        }
        printf("\\n");
        free(result);
        result = NULL;
    }
    
    return 0;
}
