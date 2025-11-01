
#include <stdio.h>
#include <stdlib.h>

static const int array[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
static const int arraySize = 10;

int getValueAtIndex(int index) {
    if (index < 0 || index >= arraySize) {
        fprintf(stderr, "Index out of bounds: %d\\n", index);
        exit(1);
    }
    return array[index];
}

int main() {
    // Test case 1
    printf("Index 0: %d\\n", getValueAtIndex(0));
    
    // Test case 2
    printf("Index 5: %d\\n", getValueAtIndex(5));
    
    // Test case 3
    printf("Index 9: %d\\n", getValueAtIndex(9));
    
    // Test case 4
    printf("Index 3: %d\\n", getValueAtIndex(3));
    
    // Test case 5
    printf("Index 7: %d\\n", getValueAtIndex(7));
    
    return 0;
}
