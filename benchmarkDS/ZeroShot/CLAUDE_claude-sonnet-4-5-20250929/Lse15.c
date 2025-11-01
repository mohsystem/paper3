
#include <stdio.h>
#include <stdbool.h>

#define ARRAY_SIZE 10

static const int id_sequence[ARRAY_SIZE] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

int getValueAtIndex(int index, bool* valid) {
    *valid = false;
    if (index < 0 || index >= ARRAY_SIZE) {
        return -1;
    }
    *valid = true;
    return id_sequence[index];
}

int getValueWithPrompt(bool* valid) {
    printf("Enter an index value: ");
    int index;
    if (scanf("%d", &index) != 1) {
        while (getchar() != '\\n');
        *valid = false;
        return -1;
    }
    return getValueAtIndex(index, valid);
}

int main() {
    bool valid;
    
    // Test cases
    printf("Test Case 1 - Index 0: %d (Valid: %d)\\n", getValueAtIndex(0, &valid), valid);
    printf("Test Case 2 - Index 5: %d (Valid: %d)\\n", getValueAtIndex(5, &valid), valid);
    printf("Test Case 3 - Index 9: %d (Valid: %d)\\n", getValueAtIndex(9, &valid), valid);
    printf("Test Case 4 - Index -1 (invalid): %d (Valid: %d)\\n", getValueAtIndex(-1, &valid), valid);
    printf("Test Case 5 - Index 10 (out of bounds): %d (Valid: %d)\\n", getValueAtIndex(10, &valid), valid);
    
    return 0;
}
