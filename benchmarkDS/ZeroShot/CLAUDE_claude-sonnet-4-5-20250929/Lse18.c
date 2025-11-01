
#include <stdio.h>
#include <stdbool.h>

static const int array[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
static const int arraySize = 10;

int getValueAtIndex(int index, bool* valid) {
    if (index < 0 || index >= arraySize) {
        *valid = false;
        return -1; // Return -1 for invalid index (secure approach)
    }
    *valid = true;
    return array[index];
}

int main() {
    bool valid;
    
    // Test case 1: Valid index in middle
    int result1 = getValueAtIndex(5, &valid);
    printf("Test 1 - Index 5: %s\\n", valid ? (printf("%d", result1), "") : "Invalid");
    if (valid) printf("%d\\n", result1); else printf("Invalid\\n");
    
    // Test case 2: Valid index at start
    int result2 = getValueAtIndex(0, &valid);
    printf("Test 2 - Index 0: ");
    if (valid) printf("%d\\n", result2); else printf("Invalid\\n");
    
    // Test case 3: Valid index at end
    int result3 = getValueAtIndex(9, &valid);
    printf("Test 3 - Index 9: ");
    if (valid) printf("%d\\n", result3); else printf("Invalid\\n");
    
    // Test case 4: Invalid index (negative)
    int result4 = getValueAtIndex(-1, &valid);
    printf("Test 4 - Index -1: ");
    if (valid) printf("%d\\n", result4); else printf("Invalid\\n");
    
    // Test case 5: Invalid index (out of bounds)
    int result5 = getValueAtIndex(15, &valid);
    printf("Test 5 - Index 15: ");
    if (valid) printf("%d\\n", result5); else printf("Invalid\\n");
    
    return 0;
}
