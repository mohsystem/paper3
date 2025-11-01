
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 10

int array[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

int* getValueAtIndex(int index) {
    // Input validation: check if index is negative
    if (index < 0) {
        printf("Error: Index cannot be negative\\n");
        return NULL;
    }
    
    // Boundary check: ensure index is within array bounds
    if (index >= ARRAY_SIZE) {
        printf("Error: Index out of bounds\\n");
        return NULL;
    }
    
    return &array[index];
}

int main() {
    // Test case 1: Valid index
    int* result1 = getValueAtIndex(5);
    printf("Test 1 - Index 5: %s\\n", result1 ? (sprintf((char[20]){0}, "%d", *result1), (char[20]){0}) : "null");
    if (result1) printf("Test 1 - Index 5: %d\\n", *result1);
    
    // Test case 2: First element
    int* result2 = getValueAtIndex(0);
    if (result2) printf("Test 2 - Index 0: %d\\n", *result2);
    
    // Test case 3: Last element
    int* result3 = getValueAtIndex(9);
    if (result3) printf("Test 3 - Index 9: %d\\n", *result3);
    
    // Test case 4: Out of bounds
    int* result4 = getValueAtIndex(15);
    printf("Test 4 - Index 15: %s\\n", result4 ? "value" : "null");
    
    // Test case 5: Negative index
    int* result5 = getValueAtIndex(-1);
    printf("Test 5 - Index -1: %s\\n", result5 ? "value" : "null");
    
    return 0;
}
