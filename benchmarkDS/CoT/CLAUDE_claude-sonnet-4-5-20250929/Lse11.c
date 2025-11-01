
#include <stdio.h>
#include <stdlib.h>

int getValueFromArray(int* array, int size, int index) {
    // Validate parameters to prevent security vulnerabilities
    if (array == NULL) {
        return -1;
    }
    
    // Validate index is within valid bounds to prevent buffer overflow
    if (index < 0 || index >= size) {
        return -1;
    }
    
    return array[index];
}

int main() {
    // Define the id_sequence array
    int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int size = sizeof(id_sequence) / sizeof(id_sequence[0]);
    
    // Test cases
    printf("Test Case 1 - Index 0: %d\\n", getValueFromArray(id_sequence, size, 0));
    printf("Test Case 2 - Index 5: %d\\n", getValueFromArray(id_sequence, size, 5));
    printf("Test Case 3 - Index 9: %d\\n", getValueFromArray(id_sequence, size, 9));
    printf("Test Case 4 - Index -1 (invalid): %d\\n", getValueFromArray(id_sequence, size, -1));
    printf("Test Case 5 - Index 15 (out of bounds): %d\\n", getValueFromArray(id_sequence, size, 15));
    
    // Read input from user
    printf("Enter an index: ");
    int index;
    
    if (scanf("%d", &index) == 1) {
        int result = getValueFromArray(id_sequence, size, index);
        return (result >= 0) ? result : 1;
    } else {
        return 1;
    }
}
