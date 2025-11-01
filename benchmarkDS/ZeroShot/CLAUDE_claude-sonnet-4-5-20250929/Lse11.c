
#include <stdio.h>
#include <stdlib.h>

int getValueFromArray(int* array, int size, int index) {
    // Secure: Check if index is within valid bounds
    if (array == NULL || index < 0 || index >= size) {
        return -1; // Return error code for invalid access
    }
    return array[index];
}

int main() {
    int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int size = sizeof(id_sequence) / sizeof(id_sequence[0]);
    
    // Test case 1: Valid index at beginning
    int index1 = 0;
    int result1 = getValueFromArray(id_sequence, size, index1);
    printf("Test 1 - Index %d: %d\\n", index1, result1);
    
    // Test case 2: Valid index in middle
    int index2 = 5;
    int result2 = getValueFromArray(id_sequence, size, index2);
    printf("Test 2 - Index %d: %d\\n", index2, result2);
    
    // Test case 3: Valid index at end
    int index3 = 9;
    int result3 = getValueFromArray(id_sequence, size, index3);
    printf("Test 3 - Index %d: %d\\n", index3, result3);
    
    // Test case 4: Invalid negative index
    int index4 = -1;
    int result4 = getValueFromArray(id_sequence, size, index4);
    printf("Test 4 - Index %d: %d\\n", index4, result4);
    
    // Test case 5: Invalid index beyond array
    int index5 = 15;
    int result5 = getValueFromArray(id_sequence, size, index5);
    printf("Test 5 - Index %d: %d\\n", index5, result5);
    
    return (result1 >= 0) ? result1 : 0;
}
