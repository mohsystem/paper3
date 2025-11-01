
#include <stdio.h>

int getValueFromArray(int array[], int size, int index) {
    if (index >= 0 && index < size) {
        return array[index];
    }
    return -1; // Return -1 for invalid index
}

int main() {
    // Test case 1
    int arr1[] = {10, 20, 30, 40, 50};
    printf("Test 1: %d\\n", getValueFromArray(arr1, 5, 2)); // Expected: 30
    
    // Test case 2
    int arr2[] = {5, 15, 25, 35, 45};
    printf("Test 2: %d\\n", getValueFromArray(arr2, 5, 0)); // Expected: 5
    
    // Test case 3
    int arr3[] = {100, 200, 300, 400, 500};
    printf("Test 3: %d\\n", getValueFromArray(arr3, 5, 4)); // Expected: 500
    
    // Test case 4
    int arr4[] = {7, 14, 21, 28, 35};
    printf("Test 4: %d\\n", getValueFromArray(arr4, 5, 3)); // Expected: 28
    
    // Test case 5
    int arr5[] = {1, 2, 3, 4, 5};
    printf("Test 5: %d\\n", getValueFromArray(arr5, 5, 1)); // Expected: 2
    
    // If you want to read from standard input and use as exit code:
    // int index;
    // scanf("%d", &index);
    // int result = getValueFromArray(arr1, 5, index);
    // return result;
    
    return 0;
}
