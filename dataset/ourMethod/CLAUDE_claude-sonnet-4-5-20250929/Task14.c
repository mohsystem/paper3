
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>

// Function to find equilibrium index where left sum equals right sum
// Returns the lowest index or -1 if not found
// Security: Input validation, overflow prevention, bounds checking, memory safety
int findEquilibriumIndex(const int* arr, int size) {
    // Input validation: NULL pointer check
    if (arr == NULL) {
        fprintf(stderr, "Error: NULL array pointer\\n");
        return -1;
    }
    
    // Input validation: Check array size is within constraints (0 < arr < 1000)
    if (size <= 0 || size >= 1000) {
        fprintf(stderr, "Error: Array size must be between 1 and 999\\n");
        return -1;
    }
    
    // Calculate total sum with overflow detection
    // Using long long to prevent integer overflow during summation
    long long totalSum = 0;
    for (int i = 0; i < size; i++) {
        // Bounds check (redundant given loop but defensive)
        if (i < 0 || i >= size) {
            fprintf(stderr, "Error: Array index out of bounds\\n");
            return -1;
        }
        
        // Check for overflow before addition
        if ((totalSum > 0 && arr[i] > LLONG_MAX - totalSum) ||
            (totalSum < 0 && arr[i] < LLONG_MIN - totalSum)) {
            fprintf(stderr, "Error: Sum calculation overflow\\n");
            return -1;
        }
        totalSum += (long long)arr[i];
    }
    
    // Iterate through array to find equilibrium index
    long long leftSum = 0;
    
    for (int i = 0; i < size; i++) {
        // Bounds check
        if (i < 0 || i >= size) {
            fprintf(stderr, "Error: Array index out of bounds\\n");
            return -1;
        }
        
        // Right sum = total - left - current element
        long long rightSum = totalSum - leftSum - (long long)arr[i];
        
        // Check if left sum equals right sum
        if (leftSum == rightSum) {
            return i;  // Return lowest index where balance occurs
        }
        
        // Update left sum with overflow check
        if ((leftSum > 0 && arr[i] > LLONG_MAX - leftSum) ||
            (leftSum < 0 && arr[i] < LLONG_MIN - leftSum)) {
            fprintf(stderr, "Error: Left sum calculation overflow\\n");
            return -1;
        }
        leftSum += (long long)arr[i];
    }
    
    // No equilibrium index found
    return -1;
}

int main(void) {
    // Test case 1: Middle equilibrium
    int test1[] = {1, 2, 3, 4, 3, 2, 1};
    int size1 = sizeof(test1) / sizeof(test1[0]);
    printf("Test 1: %d (Expected: 3)\\n", findEquilibriumIndex(test1, size1));
    
    // Test case 2: Early equilibrium
    int test2[] = {1, 100, 50, -51, 1, 1};
    int size2 = sizeof(test2) / sizeof(test2[0]);
    printf("Test 2: %d (Expected: 1)\\n", findEquilibriumIndex(test2, size2));
    
    // Test case 3: Index 0 equilibrium
    int test3[] = {20, 10, -80, 10, 10, 15, 35};
    int size3 = sizeof(test3) / sizeof(test3[0]);
    printf("Test 3: %d (Expected: 0)\\n", findEquilibriumIndex(test3, size3));
    
    // Test case 4: No equilibrium
    int test4[] = {1, 2, 3, 4, 5};
    int size4 = sizeof(test4) / sizeof(test4[0]);
    printf("Test 4: %d (Expected: -1)\\n", findEquilibriumIndex(test4, size4));
    
    // Test case 5: Single element (always equilibrium at index 0)
    int test5[] = {42};
    int size5 = sizeof(test5) / sizeof(test5[0]);
    printf("Test 5: %d (Expected: 0)\\n", findEquilibriumIndex(test5, size5));
    
    return 0;
}
