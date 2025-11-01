
#include <stdio.h>
#include <stdbool.h>

static const int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
static const int size = 10;

int getValueAtIndex(int index, bool* valid) {
    if (index < 0 || index >= size) {
        *valid = false;
        return -1; // Return -1 for invalid index
    }
    *valid = true;
    return id_sequence[index];
}

int main() {
    bool valid;
    
    // Test case 1: Valid index at beginning
    int result1 = getValueAtIndex(0, &valid);
    printf("Test 1 (index 0): %s\\n", valid ? (char[20]){0}, sprintf((char[20]){0}, "%d", result1), (char[20]){0} : "Invalid");
    printf("Test 1 (index 0): %d (valid: %d)\\n", result1, valid);
    
    // Test case 2: Valid index in middle
    int result2 = getValueAtIndex(4, &valid);
    printf("Test 2 (index 4): %d (valid: %d)\\n", result2, valid);
    
    // Test case 3: Valid index at end
    int result3 = getValueAtIndex(9, &valid);
    printf("Test 3 (index 9): %d (valid: %d)\\n", result3, valid);
    
    // Test case 4: Invalid negative index
    int result4 = getValueAtIndex(-1, &valid);
    printf("Test 4 (index -1): %d (valid: %d)\\n", result4, valid);
    
    // Test case 5: Invalid index beyond array length
    int result5 = getValueAtIndex(15, &valid);
    printf("Test 5 (index 15): %d (valid: %d)\\n", result5, valid);
    
    return 0;
}
