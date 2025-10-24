#include <stdio.h>
#include <stddef.h>

/**
 * Finds an index N in an array of integers where the sum of the integers
 * to the left of N is equal to the sum of the integers to the right of N.
 *
 * @param arr The input array of integers.
 * @param size The number of elements in the array.
 * @return The lowest index N that satisfies the condition, or -1 if no such index exists.
 */
int find_even_index(const int* arr, size_t size) {
    // Use long long to prevent potential integer overflow with large array values
    long long total_sum = 0;
    for (size_t i = 0; i < size; ++i) {
        total_sum += arr[i];
    }

    long long left_sum = 0;
    for (size_t i = 0; i < size; ++i) {
        // The right sum is the total sum minus the left sum and the current element
        long long right_sum = total_sum - left_sum - arr[i];
        
        if (left_sum == right_sum) {
            return (int)i; // Found the equilibrium index
        }
        
        // Update the left sum for the next iteration
        left_sum += arr[i];
    }

    return -1; // No such index found
}

void print_array(const int* arr, size_t size) {
    printf("Array: [");
    for (size_t i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1 ? "" : ", "));
    }
    printf("]\n");
}

int main() {
    // Test Case 1
    int test1[] = {1, 2, 3, 4, 3, 2, 1};
    size_t size1 = sizeof(test1) / sizeof(test1[0]);
    print_array(test1, size1);
    printf("Expected: 3, Actual: %d\n\n", find_even_index(test1, size1));

    // Test Case 2
    int test2[] = {1, 100, 50, -51, 1, 1};
    size_t size2 = sizeof(test2) / sizeof(test2[0]);
    print_array(test2, size2);
    printf("Expected: 1, Actual: %d\n\n", find_even_index(test2, size2));

    // Test Case 3
    int test3[] = {20, 10, -80, 10, 10, 15, 35};
    size_t size3 = sizeof(test3) / sizeof(test3[0]);
    print_array(test3, size3);
    printf("Expected: 0, Actual: %d\n\n", find_even_index(test3, size3));

    // Test Case 4
    int test4[] = {10, -80, 10, 10, 15, 35, 20};
    size_t size4 = sizeof(test4) / sizeof(test4[0]);
    print_array(test4, size4);
    printf("Expected: 6, Actual: %d\n\n", find_even_index(test4, size4));

    // Test Case 5 (No solution)
    int test5[] = {1, 2, 3, 4, 5, 6};
    size_t size5 = sizeof(test5) / sizeof(test5[0]);
    print_array(test5, size5);
    printf("Expected: -1, Actual: %d\n\n", find_even_index(test5, size5));

    return 0;
}