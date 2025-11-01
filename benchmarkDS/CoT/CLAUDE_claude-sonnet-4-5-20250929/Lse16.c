
#include <stdio.h>
#include <stdbool.h>

#define ARRAY_SIZE 10

static const int DATA_ARRAY[ARRAY_SIZE] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

bool getValueAtIndex(int index, int* result) {
    // Input validation: check for null pointer
    if (result == NULL) {
        printf("Error: Result pointer is NULL\\n");
        return false;
    }
    
    // Input validation: check for negative index
    if (index < 0) {
        printf("Error: Index cannot be negative\\n");
        return false;
    }
    
    // Bounds checking: ensure index is within array bounds
    if (index >= ARRAY_SIZE) {
        printf("Error: Index out of bounds\\n");
        return false;
    }
    
    // Safe array access
    *result = DATA_ARRAY[index];
    return true;
}

int main() {
    int value;
    
    // Test case 1: Valid index at beginning
    if (getValueAtIndex(0, &value)) {
        printf("Test 1 (index 0): %d\\n", value);
    }
    
    // Test case 2: Valid index in middle
    if (getValueAtIndex(5, &value)) {
        printf("Test 2 (index 5): %d\\n", value);
    }
    
    // Test case 3: Valid index at end
    if (getValueAtIndex(9, &value)) {
        printf("Test 3 (index 9): %d\\n", value);
    }
    
    // Test case 4: Invalid index (out of bounds)
    printf("Test 4 (index 15): ");
    if (!getValueAtIndex(15, &value)) {
        printf("Failed as expected\\n");
    }
    
    // Test case 5: Invalid index (negative)
    printf("Test 5 (index -1): ");
    if (!getValueAtIndex(-1, &value)) {
        printf("Failed as expected\\n");
    }
    
    return 0;
}
