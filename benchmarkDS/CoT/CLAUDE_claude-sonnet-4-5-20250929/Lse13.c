
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

#define ARRAY_SIZE 10

const int ID_SEQUENCE[ARRAY_SIZE] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};

int getValueAtIndex(int index, bool* success) {
    *success = false;
    
    // Input validation: check for negative index
    if (index < 0) {
        printf("Error: Index cannot be negative\\n");
        return -1;
    }
    
    // Boundary check: ensure index is within array bounds
    if (index >= ARRAY_SIZE) {
        printf("Error: Index out of bounds\\n");
        return -1;
    }
    
    *success = true;
    return ID_SEQUENCE[index];
}

void testCase(int index, const char* description) {
    printf("\\nTest: %s\\n", description);
    bool success = false;
    int result = getValueAtIndex(index, &success);
    if (success) {
        printf("Result: %d\\n", result);
    }
}

int main() {
    // Test cases
    printf("=== Running Test Cases ===\\n");
    testCase(0, "Valid index 0");
    testCase(5, "Valid index 5");
    testCase(9, "Valid index at boundary");
    testCase(-1, "Invalid negative index");
    testCase(15, "Invalid index out of bounds");
    
    // Interactive mode
    printf("\\n=== Interactive Mode ===\\n");
    printf("Enter an index value: ");
    
    int index;
    if (scanf("%d", &index) == 1) {
        bool success = false;
        int result = getValueAtIndex(index, &success);
        if (success) {
            printf("Value at index %d: %d\\n", index, result);
        }
    } else {
        printf("Error: Invalid input. Please enter an integer.\\n");
    }
    
    return 0;
}
