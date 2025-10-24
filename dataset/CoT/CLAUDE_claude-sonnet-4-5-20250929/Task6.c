
#include <stdio.h>
#include <stdlib.h>

double findUniq(double* arr, int size) {
    // Security: Validate input array
    if (arr == NULL || size < 3) {
        fprintf(stderr, "Error: Array must contain at least 3 elements\\n");
        exit(EXIT_FAILURE);
    }
    
    // Compare first three elements to determine which is unique
    if (arr[0] == arr[1]) {
        // arr[0] and arr[1] are the same
        for (int i = 2; i < size; i++) {
            if (arr[i] != arr[0]) {
                return arr[i];
            }
        }
        return arr[2];
    } else if (arr[0] == arr[2]) {
        // arr[0] and arr[2] are the same, arr[1] is unique
        return arr[1];
    } else {
        // arr[1] and arr[2] must be the same, arr[0] is unique
        return arr[0];
    }
}

int main() {
    // Test case 1
    double test1[] = {1, 1, 1, 2, 1, 1};
    printf("Test 1: %f\\n", findUniq(test1, 6));  // Expected: 2.0
    
    // Test case 2
    double test2[] = {0, 0, 0.55, 0, 0};
    printf("Test 2: %f\\n", findUniq(test2, 5));  // Expected: 0.55
    
    // Test case 3
    double test3[] = {5, 5, 5, 5, 3};
    printf("Test 3: %f\\n", findUniq(test3, 5));  // Expected: 3.0
    
    // Test case 4
    double test4[] = {10, 20, 20, 20};
    printf("Test 4: %f\\n", findUniq(test4, 4));  // Expected: 10.0
    
    // Test case 5
    double test5[] = {-1, -1, -1, -1, 0};
    printf("Test 5: %f\\n", findUniq(test5, 5));  // Expected: 0.0
    
    return 0;
}
