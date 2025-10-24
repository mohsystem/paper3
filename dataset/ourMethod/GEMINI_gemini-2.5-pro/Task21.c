#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Function to remove the smallest element from an array.
// Returns a new dynamically allocated array. The caller must free this memory.
// The size of the new array is returned via the new_size pointer.
int* removeSmallest(const int* arr, size_t size, size_t* new_size) {
    // Treat all inputs as untrusted. Validate ranges, lengths, and types.
    if (new_size == NULL) {
        // Cannot return new size, critical failure.
        return NULL;
    }

    if (arr == NULL || size == 0) {
        *new_size = 0;
        return NULL; // Return NULL for an empty array, caller should handle it.
    }

    int min_val = INT_MAX;
    size_t min_index = 0;

    for (size_t i = 0; i < size; ++i) {
        if (arr[i] < min_val) {
            min_val = arr[i];
            min_index = i;
        }
    }

    *new_size = size - 1;
    if (*new_size == 0) {
        return NULL; // Final array is empty.
    }

    // All malloc/calloc calls checked for NULL return
    int* result = (int*)malloc(*new_size * sizeof(int));
    if (result == NULL) {
        perror("Failed to allocate memory");
        *new_size = 0;
        return NULL;
    }

    size_t result_index = 0;
    // All array accesses bounds-checked
    for (size_t i = 0; i < size; ++i) {
        if (i != min_index) {
            if (result_index < *new_size) {
                result[result_index] = arr[i];
                result_index++;
            }
        }
    }

    return result;
}

// Helper function to print an array
void printArray(const char* prefix, const int* arr, size_t size) {
    printf("%s[", prefix);
    if (arr != NULL && size > 0) {
        for (size_t i = 0; i < size; ++i) {
            printf("%d", arr[i]);
            if (i < size - 1) {
                printf(", ");
            }
        }
    }
    printf("]\n");
}

void run_test_case(const int* arr, size_t size) {
    printArray("Input:  ", arr, size);
    size_t new_size = 0;
    int* result = removeSmallest(arr, size, &new_size);
    printArray("Output: ", result, new_size);
    // All allocated memory freed exactly once
    free(result);
    printf("--------------------\n");
}

int main() {
    // Test Case 1
    int test1[] = {1, 2, 3, 4, 5};
    run_test_case(test1, sizeof(test1) / sizeof(test1[0]));

    // Test Case 2
    int test2[] = {5, 3, 2, 1, 4};
    run_test_case(test2, sizeof(test2) / sizeof(test2[0]));

    // Test Case 3
    int test3[] = {2, 2, 1, 2, 1};
    run_test_case(test3, sizeof(test3) / sizeof(test3[0]));

    // Test Case 4: Single element
    int test4[] = {5};
    run_test_case(test4, sizeof(test4) / sizeof(test4[0]));

    // Test Case 5: Empty list
    int* test5 = NULL;
    run_test_case(test5, 0);

    return 0;
}