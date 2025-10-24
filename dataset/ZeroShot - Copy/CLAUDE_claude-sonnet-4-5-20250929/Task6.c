
#include <stdio.h>
#include <stdlib.h>

double findUniq(double* arr, int length) {
    if (arr == NULL || length < 3) {
        fprintf(stderr, "Array must contain at least 3 numbers\\n");
        exit(1);
    }
    
    // Compare first three elements to determine which is the unique one
    if (arr[0] == arr[1]) {
        // arr[0] and arr[1] are the same, so they represent the repeated value
        for (int i = 2; i < length; i++) {
            if (arr[i] != arr[0]) {
                return arr[i];
            }
        }
        return arr[0]; // Should not reach here given problem constraints
    } else if (arr[0] == arr[2]) {
        // arr[0] and arr[2] are the same, so arr[1] is unique
        return arr[1];
    } else {
        // arr[1] and arr[2] must be the same, so arr[0] is unique
        return arr[0];
    }
}

int main() {
    // Test case 1
    double test1[] = {1, 1, 1, 2, 1, 1};
    double result1 = findUniq(test1, 6);
    printf("Test 1: %f (Expected: 2.0)\\n", result1);
    
    // Test case 2
    double test2[] = {0, 0, 0.55, 0, 0};
    double result2 = findUniq(test2, 5);
    printf("Test 2: %f (Expected: 0.55)\\n", result2);
    
    // Test case 3
    double test3[] = {5, 5, 5, 5, 10};
    double result3 = findUniq(test3, 5);
    printf("Test 3: %f (Expected: 10.0)\\n", result3);
    
    // Test case 4
    double test4[] = {3.14, 2.71, 2.71, 2.71};
    double result4 = findUniq(test4, 4);
    printf("Test 4: %f (Expected: 3.14)\\n", result4);
    
    // Test case 5
    double test5[] = {-1, -1, -1, -5, -1, -1, -1};
    double result5 = findUniq(test5, 7);
    printf("Test 5: %f (Expected: -5.0)\\n", result5);
    
    return 0;
}
