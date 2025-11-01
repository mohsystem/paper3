
#include <stdio.h>
#include <stdlib.h>

int array[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
int arraySize = 10;

int getValueAtIndex(int index, int* error) {
    if (index >= 0 && index < arraySize) {
        *error = 0;
        return array[index];
    }
    *error = 1;
    return -1;
}

int main() {
    int error;
    int result;
    
    // Test case 1
    result = getValueAtIndex(5, &error);
    if (!error) {
        printf("Index 5: %d\\n", result);
    } else {
        printf("Error: Index out of bounds\\n");
    }
    
    // Test case 2
    result = getValueAtIndex(0, &error);
    if (!error) {
        printf("Index 0: %d\\n", result);
    } else {
        printf("Error: Index out of bounds\\n");
    }
    
    // Test case 3
    result = getValueAtIndex(9, &error);
    if (!error) {
        printf("Index 9: %d\\n", result);
    } else {
        printf("Error: Index out of bounds\\n");
    }
    
    // Test case 4
    result = getValueAtIndex(3, &error);
    if (!error) {
        printf("Index 3: %d\\n", result);
    } else {
        printf("Error: Index out of bounds\\n");
    }
    
    // Test case 5
    result = getValueAtIndex(10, &error);
    if (!error) {
        printf("Index 10: %d\\n", result);
    } else {
        printf("Error: Index out of bounds\\n");
    }
    
    return 0;
}
