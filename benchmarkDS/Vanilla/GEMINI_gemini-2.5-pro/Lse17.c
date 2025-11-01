#include <stdio.h>

int getValueAtIndex(int arr[], int index) {
    return arr[index];
}

int main() {
    int myArray[] = {10, 20, 30, 40, 50, 60};

    // Test Case 1
    int index1 = 0;
    printf("Value at index %d: %d\n", index1, getValueAtIndex(myArray, index1));

    // Test Case 2
    int index2 = 2;
    printf("Value at index %d: %d\n", index2, getValueAtIndex(myArray, index2));

    // Test Case 3
    int index3 = 4;
    printf("Value at index %d: %d\n", index3, getValueAtIndex(myArray, index3));

    // Test Case 4
    int index4 = 5;
    printf("Value at index %d: %d\n", index4, getValueAtIndex(myArray, index4));

    // Test Case 5
    int index5 = 1;
    printf("Value at index %d: %d\n", index5, getValueAtIndex(myArray, index5));

    return 0;
}