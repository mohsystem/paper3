
#include <stdio.h>
#include <stdlib.h>

const int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
const int id_sequence_size = 10;

int getValueAtIndex(int index, int* error) {
    if (index >= 0 && index < id_sequence_size) {
        *error = 0;
        return id_sequence[index];
    }
    *error = 1;
    return -1;
}

int main() {
    int error;
    int result;
    
    // Test case 1
    result = getValueAtIndex(0, &error);
    if (!error) {
        printf("Test 1 - Index 0: %d\\n", result);
    } else {
        printf("Test 1 - Error: Index out of bounds\\n");
    }
    
    // Test case 2
    result = getValueAtIndex(5, &error);
    if (!error) {
        printf("Test 2 - Index 5: %d\\n", result);
    } else {
        printf("Test 2 - Error: Index out of bounds\\n");
    }
    
    // Test case 3
    result = getValueAtIndex(9, &error);
    if (!error) {
        printf("Test 3 - Index 9: %d\\n", result);
    } else {
        printf("Test 3 - Error: Index out of bounds\\n");
    }
    
    // Test case 4
    result = getValueAtIndex(-1, &error);
    if (!error) {
        printf("Test 4 - Index -1: %d\\n", result);
    } else {
        printf("Test 4 - Error: Index out of bounds\\n");
    }
    
    // Test case 5
    result = getValueAtIndex(15, &error);
    if (!error) {
        printf("Test 5 - Index 15: %d\\n", result);
    } else {
        printf("Test 5 - Error: Index out of bounds\\n");
    }
    
    return 0;
}
