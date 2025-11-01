
#include <stdio.h>
#include <stdlib.h>

int getValueAtIndex(int index, int* array, int arraySize, int* error) {
    *error = 0;
    if (index < 0 || index >= arraySize) {
        *error = 1;
        return -1;
    }
    return array[index];
}

int main() {
    int testArray[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int arraySize = sizeof(testArray) / sizeof(testArray[0]);
    int error;
    int result;
    
    // Test case 1: Valid index at beginning
    result = getValueAtIndex(0, testArray, arraySize, &error);
    if (error) {
        printf("Test 1 Error: Index out of bounds\\n");
    } else {
        printf("Test 1 - Index 0: %d\\n", result);
    }
    
    // Test case 2: Valid index in middle
    result = getValueAtIndex(5, testArray, arraySize, &error);
    if (error) {
        printf("Test 2 Error: Index out of bounds\\n");
    } else {
        printf("Test 2 - Index 5: %d\\n", result);
    }
    
    // Test case 3: Valid index at end
    result = getValueAtIndex(9, testArray, arraySize, &error);
    if (error) {
        printf("Test 3 Error: Index out of bounds\\n");
    } else {
        printf("Test 3 - Index 9: %d\\n", result);
    }
    
    // Test case 4: Invalid negative index
    result = getValueAtIndex(-1, testArray, arraySize, &error);
    if (error) {
        printf("Test 4 Error: Index out of bounds\\n");
    } else {
        printf("Test 4 - Index -1: %d\\n", result);
    }
    
    // Test case 5: Invalid index beyond array length
    result = getValueAtIndex(15, testArray, arraySize, &error);
    if (error) {
        printf("Test 5 Error: Index out of bounds\\n");
    } else {
        printf("Test 5 - Index 15: %d\\n", result);
    }
    
    return 0;
}
