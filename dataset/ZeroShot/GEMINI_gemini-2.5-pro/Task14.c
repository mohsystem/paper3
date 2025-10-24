#include <stdio.h>

/**
 * Finds an index in an array of integers where the sum of integers to the left
 * of the index is equal to the sum of integers to the right.
 *
 * @param arr The input array of integers.
 * @param size The number of elements in the array.
 * @return The lowest index N where the condition is met, or -1 if no such index exists.
 */
int find_even_index(const int* arr, int size) {
    // Use long long for sums to prevent potential integer overflow, a security best practice.
    long long total_sum = 0;
    for (int i = 0; i < size; i++) {
        total_sum += arr[i];
    }

    long long left_sum = 0;
    for (int i = 0; i < size; i++) {
        // The sum of the right side is the total sum minus the left sum and the current element.
        long long right_sum = total_sum - left_sum - arr[i];

        if (left_sum == right_sum) {
            return i;
        }

        left_sum += arr[i];
    }
    
    return -1; // Return -1 if no such index is found.
}

// Helper function to print an array and run a test case
void run_test_case(const char* name, const int* arr, int size) {
    printf("%s Array: {", name);
    for (int i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
    }
    printf("} -> Index: %d\n", find_even_index(arr, size));
}

int main() {
    // Test Case 1: Example from prompt
    int test1[] = {1, 2, 3, 4, 3, 2, 1};
    run_test_case("Test Case 1:", test1, sizeof(test1)/sizeof(test1[0]));

    // Test Case 2: Example from prompt
    int test2[] = {1, 100, 50, -51, 1, 1};
    run_test_case("Test Case 2:", test2, sizeof(test2)/sizeof(test2[0]));

    // Test Case 3: Example from prompt (index 0)
    int test3[] = {20, 10, -80, 10, 10, 15, 35};
    run_test_case("Test Case 3:", test3, sizeof(test3)/sizeof(test3[0]));
    
    // Test Case 4: Index is the last element
    int test4[] = {10, -80, 10, 10, 15, 35, 20};
    run_test_case("Test Case 4:", test4, sizeof(test4)/sizeof(test4[0]));

    // Test Case 5: No solution
    int test5[] = {1, 2, 3, 4, 5, 6};
    run_test_case("Test Case 5:", test5, sizeof(test5)/sizeof(test5[0]));

    return 0;
}