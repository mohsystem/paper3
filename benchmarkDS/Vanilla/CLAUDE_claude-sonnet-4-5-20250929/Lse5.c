
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* getElementAtIndex(int index) {
    static const char* elements[] = {"apple", "banana", "cherry", "date", "elderberry"};
    int size = 5;
    
    if (index >= 0 && index < size) {
        return elements[index];
    } else {
        return "Index out of bounds";
    }
}

int main(int argc, char* argv[]) {
    // Test cases
    printf("Test case 1: %s\\n", getElementAtIndex(0));
    printf("Test case 2: %s\\n", getElementAtIndex(2));
    printf("Test case 3: %s\\n", getElementAtIndex(4));
    printf("Test case 4: %s\\n", getElementAtIndex(-1));
    printf("Test case 5: %s\\n", getElementAtIndex(10));
    
    // Command line argument handling
    if (argc > 1) {
        int index = atoi(argv[1]);
        printf("Element at index %d: %s\\n", index, getElementAtIndex(index));
    }
    
    return 0;
}
