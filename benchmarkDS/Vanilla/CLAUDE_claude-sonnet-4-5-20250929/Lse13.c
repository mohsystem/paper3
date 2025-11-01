
#include <stdio.h>

int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
int id_sequence_size = 10;

int getValueAtIndex(int index) {
    if (index >= 0 && index < id_sequence_size) {
        return id_sequence[index];
    }
    return -1; // Return -1 for invalid index
}

int main() {
    // Test case 1
    int index1 = 0;
    printf("Index %d: %d\\n", index1, getValueAtIndex(index1));
    
    // Test case 2
    int index2 = 5;
    printf("Index %d: %d\\n", index2, getValueAtIndex(index2));
    
    // Test case 3
    int index3 = 9;
    printf("Index %d: %d\\n", index3, getValueAtIndex(index3));
    
    // Test case 4
    int index4 = -1;
    printf("Index %d: %d\\n", index4, getValueAtIndex(index4));
    
    // Test case 5
    int index5 = 10;
    printf("Index %d: %d\\n", index5, getValueAtIndex(index5));
    
    return 0;
}
