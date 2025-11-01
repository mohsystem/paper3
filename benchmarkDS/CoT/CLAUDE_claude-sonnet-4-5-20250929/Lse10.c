
#include <stdio.h>
#include <stdlib.h>

int getValueFromArray(int* array, int size, int index) {
    // Security: Validate index to prevent out-of-bounds access
    if (array == NULL || index < 0 || index >= size) {
        return -1; // Return error code for invalid access
    }
    return array[index];
}

int main() {
    int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int size = sizeof(id_sequence) / sizeof(id_sequence[0]);
    
    // Test cases
    printf("Test Case 1 (index 0): %d\\n", getValueFromArray(id_sequence, size, 0));
    printf("Test Case 2 (index 5): %d\\n", getValueFromArray(id_sequence, size, 5));
    printf("Test Case 3 (index 9): %d\\n", getValueFromArray(id_sequence, size, 9));
    printf("Test Case 4 (index -1): %d\\n", getValueFromArray(id_sequence, size, -1));
    printf("Test Case 5 (index 15): %d\\n", getValueFromArray(id_sequence, size, 15));
    
    // Read from standard input
    printf("Enter an index: ");
    int index;
    if (scanf("%d", &index) == 1) {
        int result = getValueFromArray(id_sequence, size, index);
        return (result >= 0) ? result : 1;
    }
    return 1;
}
