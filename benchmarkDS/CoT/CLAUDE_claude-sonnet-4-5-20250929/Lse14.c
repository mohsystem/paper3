
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define ARRAY_SIZE 10

int id_sequence[ARRAY_SIZE] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};

int* getValueAtIndex(int index) {
    // Validate index bounds
    if (index < 0 || index >= ARRAY_SIZE) {
        printf("Error: Index out of bounds. Valid range: 0-%d\\n", ARRAY_SIZE - 1);
        return NULL;
    }
    return &id_sequence[index];
}

int* getUserInputAndGetValue() {
    int index;
    printf("Enter an index value (0-%d): ", ARRAY_SIZE - 1);
    
    if (scanf("%d", &index) != 1) {
        printf("Error: Invalid input. Please enter a valid integer.\\n");
        // Clear input buffer
        while (getchar() != '\\n');
        return NULL;
    }
    
    return getValueAtIndex(index);
}

int main() {
    // Test case 1: Valid index at beginning
    int* result1 = getValueAtIndex(0);
    printf("Test 1 - Index 0: %s\\n", result1 ? (sprintf((char[20]){0}, "%d", *result1), (char[20]){0}) : "null");
    if (result1) printf("Test 1 - Index 0: %d\\n", *result1);
    else printf("Test 1 - Index 0: null\\n");
    
    // Test case 2: Valid index in middle
    int* result2 = getValueAtIndex(5);
    if (result2) printf("Test 2 - Index 5: %d\\n", *result2);
    else printf("Test 2 - Index 5: null\\n");
    
    // Test case 3: Valid index at end
    int* result3 = getValueAtIndex(9);
    if (result3) printf("Test 3 - Index 9: %d\\n", *result3);
    else printf("Test 3 - Index 9: null\\n");
    
    // Test case 4: Invalid negative index
    int* result4 = getValueAtIndex(-1);
    if (result4) printf("Test 4 - Index -1: %d\\n", *result4);
    else printf("Test 4 - Index -1: null\\n");
    
    // Test case 5: Invalid index beyond bounds
    int* result5 = getValueAtIndex(15);
    if (result5) printf("Test 5 - Index 15: %d\\n", *result5);
    else printf("Test 5 - Index 15: null\\n");
    
    return 0;
}
