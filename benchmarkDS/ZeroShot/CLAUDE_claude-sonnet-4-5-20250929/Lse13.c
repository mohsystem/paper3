
#include <stdio.h>
#include <stdbool.h>

int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
int id_sequence_size = sizeof(id_sequence) / sizeof(id_sequence[0]);

bool getValueAtIndex(int index, int* result) {
    if (index < 0 || index >= id_sequence_size) {
        return false; // Return false for invalid index
    }
    *result = id_sequence[index];
    return true; // Return true for valid index
}

int main() {
    int result;
    
    // Test case 1: Valid index at start
    if (getValueAtIndex(0, &result)) {
        printf("Test 1 (index 0): %d\\n", result);
    } else {
        printf("Test 1 (index 0): null\\n");
    }
    
    // Test case 2: Valid index in middle
    if (getValueAtIndex(5, &result)) {
        printf("Test 2 (index 5): %d\\n", result);
    } else {
        printf("Test 2 (index 5): null\\n");
    }
    
    // Test case 3: Valid index at end
    if (getValueAtIndex(9, &result)) {
        printf("Test 3 (index 9): %d\\n", result);
    } else {
        printf("Test 3 (index 9): null\\n");
    }
    
    // Test case 4: Invalid negative index
    if (getValueAtIndex(-1, &result)) {
        printf("Test 4 (index -1): %d\\n", result);
    } else {
        printf("Test 4 (index -1): null\\n");
    }
    
    // Test case 5: Invalid out of bounds index
    if (getValueAtIndex(10, &result)) {
        printf("Test 5 (index 10): %d\\n", result);
    } else {
        printf("Test 5 (index 10): null\\n");
    }
    
    return 0;
}
