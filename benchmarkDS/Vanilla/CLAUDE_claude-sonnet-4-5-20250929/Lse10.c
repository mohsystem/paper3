
#include <stdio.h>
#include <stdlib.h>

int getValueFromArray(int array[], int size, int index) {
    if (index >= 0 && index < size) {
        return array[index];
    }
    return -1; // Return -1 for invalid index
}

int main() {
    int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int size = sizeof(id_sequence) / sizeof(id_sequence[0]);
    
    // Test case 1: Valid index 0
    int index1 = 0;
    int result1 = getValueFromArray(id_sequence, size, index1);
    printf("Test 1 - Index %d: %d\\n", index1, result1);
    
    // Test case 2: Valid index 5
    int index2 = 5;
    int result2 = getValueFromArray(id_sequence, size, index2);
    printf("Test 2 - Index %d: %d\\n", index2, result2);
    
    // Test case 3: Valid index 9 (last element)
    int index3 = 9;
    int result3 = getValueFromArray(id_sequence, size, index3);
    printf("Test 3 - Index %d: %d\\n", index3, result3);
    
    // Test case 4: Invalid index -1
    int index4 = -1;
    int result4 = getValueFromArray(id_sequence, size, index4);
    printf("Test 4 - Index %d: %d\\n", index4, result4);
    
    // Test case 5: Invalid index 10 (out of bounds)
    int index5 = 10;
    int result5 = getValueFromArray(id_sequence, size, index5);
    printf("Test 5 - Index %d: %d\\n", index5, result5);
    
    return result1;
}
