#include <stdio.h>
#include <stddef.h>

/**
 * @brief Finds an index N in an array where the sum of integers to the left of N
 * is equal to the sum of integers to the right of N.
 *
 * @param arr A pointer to a constant integer array.
 * @param size The number of elements in the array.
 * @return The lowest index N that satisfies the condition, or -1 if no such index exists.
 */
int find_even_index(const int* arr, size_t size) {
    if (arr == NULL) {
        return -1; // Handle null pointer input
    }

    // Use long long to prevent potential integer overflow during summation.
    long long total_sum = 0;
    for (size_t i = 0; i < size; ++i) {
        total_sum += arr[i];
    }

    long long left_sum = 0;
    for (size_t i = 0; i < size; ++i) {
        // The sum of the right side is the total sum minus the left sum and the current element.
        long long right_sum = total_sum - left_sum - arr[i];
        
        if (left_sum == right_sum) {
            return (int)i; // Found the equilibrium index.
        }

        // Add the current element to the left sum for the next iteration.
        left_sum += arr[i];
    }

    // If the loop completes, no such index was found.
    return -1;
}

void run_test_case(const char* name, const int* arr, size_t size, int expected) {
    int result = find_even_index(arr, size);
    printf("Test Case: %s\n", name);
    printf("Array: {");
    for (size_t i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1 ? "" : ", "));
    }
    printf("}\n");
    printf("Expected: %d, Got: %d\n", expected, result);
    printf("%s\n\n", (result == expected ? "PASSED" : "FAILED"));
}

int main() {
    // Test Case 1: Standard case from prompt
    int arr1[] = {1, 2, 3, 4, 3, 2, 1};
    size_t size1 = sizeof(arr1) / sizeof(arr1[0]);
    run_test_case("Example 1", arr1, size1, 3);

    // Test Case 2: Another standard case from prompt
    int arr2[] = {1, 100, 50, -51, 1, 1};
    size_t size2 = sizeof(arr2) / sizeof(arr2[0]);
    run_test_case("Example 2", arr2, size2, 1);

    // Test Case 3: Equilibrium at the first index
    int arr3[] = {20, 10, -80, 10, 10, 15, 35};
    size_t size3 = sizeof(arr3) / sizeof(arr3[0]);
    run_test_case("Index 0", arr3, size3, 0);

    // Test Case 4: Equilibrium at the last index
    int arr4[] = {10, -80, 10, 10, 15, 35, 20};
    size_t size4 = sizeof(arr4) / sizeof(arr4[0]);
    run_test_case("Last Index", arr4, size4, 6);

    // Test Case 5: No solution exists
    int arr5[] = {1, 2, 3, 4, 5, 6};
    size_t size5 = sizeof(arr5) / sizeof(arr5[0]);
    run_test_case("No Solution", arr5, size5, -1);

    return 0;
}