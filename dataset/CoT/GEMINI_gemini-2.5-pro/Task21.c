#include <stdio.h>
#include <stdlib.h>
#include <limits.h> // For INT_MAX

/**
 * Given an array of integers, create a new array with the smallest value removed.
 * Do not mutate the original array.
 * If there are multiple elements with the same value, remove the one with the lowest index.
 * If you get an empty array, return an empty array (NULL).
 * The caller is responsible for freeing the returned array.
 *
 * @param arr The input array of integers.
 * @param size The number of elements in the input array.
 * @param new_size A pointer to a size_t variable to store the size of the new array.
 * @return A new dynamically allocated array with the smallest value removed, or NULL for empty input.
 */
int* remove_smallest(const int* arr, size_t size, size_t* new_size) {
    if (arr == NULL || size == 0) {
        *new_size = 0;
        return NULL;
    }

    int min_value = INT_MAX;
    int min_index = 0;

    for (size_t i = 0; i < size; ++i) {
        if (arr[i] < min_value) {
            min_value = arr[i];
            min_index = i;
        }
    }

    *new_size = size - 1;
    int* result = (int*)malloc(*new_size * sizeof(int));
    if (result == NULL) {
        // Handle malloc failure
        *new_size = 0;
        return NULL;
    }

    int result_index = 0;
    for (size_t i = 0; i < size; ++i) {
        if (i != (size_t)min_index) {
            result[result_index++] = arr[i];
        }
    }

    return result;
}

// Helper function to print an array
void print_array(const char* label, const int* arr, size_t size) {
    printf("%s[", label);
    for (size_t i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
    }
    printf("]\n");
}

int main() {
    // Test Cases
    int tc1[] = {1, 2, 3, 4, 5};
    int tc2[] = {5, 3, 2, 1, 4};
    int tc3[] = {2, 2, 1, 2, 1};
    int* tc4 = NULL; // Empty case
    int tc5[] = {3, 3, 3, 3, 3};

    const int* test_cases[] = {tc1, tc2, tc3, tc4, tc5};
    size_t sizes[] = {5, 5, 5, 0, 5};
    
    for (int i = 0; i < 5; ++i) {
        const int* original = test_cases[i];
        size_t original_size = sizes[i];
        
        size_t result_size;
        int* result = remove_smallest(original, original_size, &result_size);
        
        printf("Test Case %d:\n", i + 1);
        print_array("Original: ", original, original_size);
        print_array("Result:   ", result, result_size);
        printf("\n");
        
        // Free the dynamically allocated memory for the result
        free(result);
    }

    return 0;
}