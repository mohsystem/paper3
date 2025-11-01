
#include <stdio.h>
#include <stdlib.h>

int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
int id_sequence_size = 10;

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
    
    // Test case 1: Valid index at beginning
    result = getValueAtIndex(0, &error);
    if (!error) {
        printf("Test 1 (index 0): %d\\n", result);
    } else {
        printf("Test 1 failed: Index out of bounds\\n");
    }
    
    // Test case 2: Valid index in middle
    result = getValueAtIndex(5, &error);
    if (!error) {
        printf("Test 2 (index 5): %d\\n", result);
    } else {
        printf("Test 2 failed: Index out of bounds\\n");
    }
    
    // Test case 3: Valid index at end
    result = getValueAtIndex(9, &error);
    if (!error) {
        printf("Test 3 (index 9): %d\\n", result);
    } else {
        printf("Test 3 failed: Index out of bounds\\n");
    }
    
    // Test case 4: Invalid negative index
    result = getValueAtIndex(-1, &error);
    if (!error) {
        printf("Test 4 (index -1): %d\\n", result);
    } else {
        printf("Test 4 failed: Index out of bounds\\n");
    }
    
    // Test case 5: Invalid index beyond array length
    result = getValueAtIndex(15, &error);
    if (!error) {
        printf("Test 5 (index 15): %d\\n", result);
    } else {
        printf("Test 5 failed: Index out of bounds\\n");
    }
    
    return 0;
}
