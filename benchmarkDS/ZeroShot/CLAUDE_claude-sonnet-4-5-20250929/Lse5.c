
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* getElementAtIndex(int index) {
    static const char* elements[] = {"apple", "banana", "cherry", "date", "elderberry"};
    static const int size = 5;
    static char error_msg[50];
    
    if (index < 0 || index >= size) {
        strcpy(error_msg, "Error: Index out of bounds");
        return error_msg;
    }
    
    return elements[index];
}

int main(int argc, char* argv[]) {
    // Test cases
    printf("Test Case 1: %s\\n", getElementAtIndex(0));
    printf("Test Case 2: %s\\n", getElementAtIndex(2));
    printf("Test Case 3: %s\\n", getElementAtIndex(4));
    printf("Test Case 4: %s\\n", getElementAtIndex(-1));
    printf("Test Case 5: %s\\n", getElementAtIndex(10));
    
    // Command line argument processing
    if (argc > 1) {
        char* endptr;
        long index = strtol(argv[1], &endptr, 10);
        
        if (*endptr != '\\0' || endptr == argv[1]) {
            printf("Error: Invalid number format\\n");
        } else {
            printf("\\nCommand line result: %s\\n", getElementAtIndex((int)index));
        }
    }
    
    return 0;
}
