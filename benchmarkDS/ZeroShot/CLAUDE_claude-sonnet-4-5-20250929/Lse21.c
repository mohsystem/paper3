
#include <stdio.h>
#include <stdlib.h>

int addThousand(int value) {
    return value + 1000;
}

int main(int argc, char* argv[]) {
    // Test cases
    int testCases[] = {100, -500, 0, 9999, -1000};
    
    for (int i = 0; i < 5; i++) {
        int result = addThousand(testCases[i]);
        printf("Input: %d, Output: %d\\n", testCases[i], result);
    }
    
    // Command line argument handling
    if (argc > 1) {
        char* endptr;
        long value = strtol(argv[1], &endptr, 10);
        
        if (*endptr != '\\0' || endptr == argv[1]) {
            fprintf(stderr, "Invalid input. Please provide a valid integer.\\n");
            return 1;
        }
        
        int result = addThousand((int)value);
        printf("%d\\n", result);
    }
    
    return 0;
}
